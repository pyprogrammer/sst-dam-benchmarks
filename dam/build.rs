use std::env;
use std::path::PathBuf;

fn main() {
    let path = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap())
        .parent()
        .unwrap()
        .join("common");

    cxx_build::bridge("src/main.rs")
        .file(path.join("workloads.h"))
        .flag_if_supported("-std=c++14")
        .flag("-O2")
        .compile("bindings");

    println!("cargo:rerun-if-changed=src/main.rs");
}
