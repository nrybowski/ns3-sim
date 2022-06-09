#! /bin/bash -xe

git submodule init
git submodule update
git -C bird apply < bird.patch
cargo run --manifest-path=wrapper/Cargo.toml -- -n house.ntf
