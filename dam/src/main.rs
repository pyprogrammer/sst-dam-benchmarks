#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("/home/nzhang/sst-dam-benchmarks/common/workloads.h");

        fn compute_fibonacci(value: u64) -> u64;
    }
}

fn main() {
    let result = ffi::compute_fibonacci(13);
    println!("Hello, world! {:?}", result);
}
