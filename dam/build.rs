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
        .flag("-O3")
        .flag(format!("-I{}", path.as_path().display()).as_str())
        .compile("bindings");
    println!("cargo:rerun-if-changed=src/main.rs");
}
