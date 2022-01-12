use std_semaphore::Semaphore;
use std::{thread, panic, process::Command, env, fs};
use std::thread::*;
use std::sync::Arc;

use structopt::StructOpt;

#[macro_export]
macro_rules! info {
    ( $( $x:expr )? ) => {
        $(
            println!("[INFO][wrapper] {}", $x);
        )?
    }
}

#[macro_export]
macro_rules! error {
    ( $( $x:expr )? ) => {
        $(
            println!("[ERROR][wrapper] {}", $x);
        )?
    }
}

fn run_ns3(opts: Cli, pwd: String) {
    let udp = match opts.udp {
        Some(value) => value,
        None => false
    };
    let mut cli: String = format!(
        "--ntf={ntf} --check=true --runtime={runtime} --spt_delay={spt} --udp={udp}", 
        ntf=opts.ntf,
        runtime=opts.runtime,
        spt=opts.spt,
        udp=udp
    );
    if !opts.failures.is_none() {
        cli = format!("{cli} --failures={failures}", cli=cli, failures=opts.failures.unwrap());
    }
    if !opts.pcap.is_none() {
        cli = format!("{cli} --pcap={pcap}", cli=cli, pcap=opts.pcap.unwrap());
    }
    println!("{}", opts.output);
    fs::create_dir_all(&opts.output).expect("Can't create output directories");

    info!("Starting NS3 container. Giving hand to NS3.");
    let child = Command::new("docker")
                        .arg("run")
                        .arg("--rm")
                        .arg("-v")
                        .arg(format!("{output}:/data/output", output=opts.output))
                        .arg("-v")
                        .arg(format!("{pwd}/bird:/data/bird", pwd=pwd))
                        .arg("-v")
                        .arg(format!("{pwd}/myscripts:/home/ns3dce/dce-linux-dev/source/dce-linux-dev/myscripts", pwd=pwd))
                        .arg("-v")
                        .arg(format!("{pwd}/inputs:/data/inputs", pwd=pwd))
                        .arg("-v")
                        .arg(format!("{pwd}/udp_ping:/data/my_exe", pwd=pwd))
                        .arg("-e")
                        .arg("NS_LOG=NtfTopoHelper=all:BlackholeErrorModel=all")
                        .arg("-e")
                        .arg("DCE_PATH=/data/bird:/data/my_exe:${DCE_PATH}")
                        .arg("ns3-bird")
                        .arg(cli)
                        //.stdout(Stdio::piped())
                        .spawn();

    child.unwrap().wait().unwrap();
    /*match child {
        Ok(mut child) => {
            match child.stdout {
                Some(ref mut stdout) => {
                    let mut stdout = stdout;
                    io::copy(&mut stdout, &mut io::stdout());
                },
                _ => {} // TODO
            }
            child.wait();
        },
        Err(_) => {} // TODO
    }*/



    /*match output {
        // unstable                            
        /*Ok(output) => match output.status.exit_ok() {
            Ok(_) => {},
            Err(_status_code) => println!("{:#?}", String::from_utf8(output.stderr))
        },*/
        Ok(output) => if !output.status.success() {
            error!(format!("{:#?}", String::from_utf8(output.stderr)));
        } else {
            error!(format!("{:#?}", String::from_utf8(output.stdout)));
        },
        Err(e) => {error!(e);}
    }*/
    
    info!("NS3 finished");
}

fn save_output(dest: String) {
}

#[derive(StructOpt, Clone)]
struct Cli {
    #[structopt(short, long)]
    ntf: String,
    #[structopt(short, long, help="NTF file specifying the link failures to simulate", required_if("single", "true"))]
    failures: Option<String>,
    #[structopt(short, long, help="Simulate the failure of each specified link one at a time. Used with -f")]
    single: Option<bool>,
    #[structopt(short, long, default_value="300")]
    runtime: u32,
    #[structopt(short, long)]
    pcap: Option<bool>,
    #[structopt(skip)]
    output: String,
    #[structopt(long, default_value="100")]
    spt: u32,
    #[structopt(long, help="Enable udp ping between nodes.")]
    udp: Option<bool>,
}

fn main() {
    let mut opt = Cli::from_args();
    let pwd: String = env::var("PWD").unwrap();
    opt.output = format!("{pwd}/output/{ntf}", pwd=pwd, ntf=opt.ntf);

    let nproc_cmd = Command::new("nproc")
                        .output()
                        .expect("Can't execute <nproc>");
    let procs = match String::from_utf8(nproc_cmd.stdout).expect("Can't parse <nproc> output").split("\n").next() {
        Some(nproc) => nproc.parse().expect("Can't parse <nproc> into isize"),
        None => Err(()).expect("Integer not found in cmdline stdout")
    };

    let single = match opt.single {
        Some(value) => value,
        None => false
    };

    let mut child = Command::new(format!("{pwd}containers/build.sh", pwd=pwd))
                        .current_dir(&pwd)
                        .spawn()
                        .expect("Failed to rebuild the containers.");
    let exit_code = child.wait()
         .expect("Failed to wait the containers rebuild");
    assert!(exit_code.success());

    if single && !opt.failures.is_none() {
        let new_opts = opt.clone();
        let filename = format!("{pwd}/inputs/{failures}", pwd=pwd, failures=opt.failures.unwrap());
        let content = fs::read_to_string(filename);
        match content {
            Err(_) => {}, // TODO
            Ok(content) => {
                println!("{:#?}", content);
                let mut ths: Vec<JoinHandle<()>> = Vec::new();
                let sem = Arc::new(Semaphore::new(procs));
                let pwd = Arc::new(pwd);
                
                let lines: Vec<&str> = content.split('\n').collect();
                for (idx, line) in lines.iter().enumerate() {

                    // skip last empty line
                    if *line == "" {continue;}
                    
                    // create tempfile
                    let tmpfile_output = Command::new("mktemp")
                                          .arg("-p")
                                          .arg(format!("{pwd}/inputs/", pwd=pwd))
                                          .arg("tmp.XXXXXX.ntf")
                                          .output()
                                          .expect("Failed to create tempfile");

                    // get tempfile name
                    let mut tmpfile = String::from_utf8(tmpfile_output.stdout)
                                            .expect("Can't read tempfile name.");
                    
                    // remove "\n" from the filename
                    tmpfile.pop();                     

                    // dump current line in tempfile
                    fs::write(&tmpfile, line)
                        .expect("Can't write NTF line into tmpfile.");

                    // create new cli configuration with tempfile as failures file
                    let mut my_opts = new_opts.clone();
                    my_opts.output = format!("{output}/{idx}", output=my_opts.output, idx=idx);
                    my_opts.failures = Some(
                        std::path::Path::new(&tmpfile)
                                        .file_name()
                                        .unwrap()
                                        .to_os_string()
                                        .into_string()
                                        .expect("Can't extract temp file name from full path")
                    );

                    // launch NS3 instance
                    let my_sem = sem.clone();
                    let my_pwd = pwd.clone();
                    ths.push(thread::spawn(move || {
                        my_sem.acquire();
                        run_ns3(my_opts, my_pwd.to_string());
                        fs::remove_file(&tmpfile).expect("Can't remove tempfile");
                        my_sem.release();
                    }));
                }

                // join all the threads in order
                for th in ths {
                    match th.join() {
                        Ok(_) => {},
                        Err(e) => panic::resume_unwind(e)
                    }
                }
            }
        }

    } else {
        run_ns3(opt, pwd);
    }
}
