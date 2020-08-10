#![allow(unused_imports)]
use cmake;
use cmake::Config;
use std::env;
use std::path::PathBuf;
extern crate bindgen;

#[cfg(target_os = "windows")]
fn main() {}

#[cfg(not(target_os = "windows"))]
fn main() {
    let out_dir = env::var("OUT_DIR").unwrap();
    let out_path = PathBuf::from(out_dir);

    #[cfg(not(feature = "force_32"))]
    {
        let _ittnotify_64 = Config::new("./")
            .generator("Unix Makefiles")
            .no_build_target(true)
            .build();

        println!("cargo:rustc-link-search={}/build/bin/", out_path.display());
        println!("cargo:rustc-link-lib=static=ittnotify64");
    }

    #[cfg(feature = "force_32")]
    #[cfg(not(any(target_os = "ios", target_os = "macos")))]
    {
        let _ittnotify_32 = Config::new("./")
            .generator("Unix Makefiles")
            .define("FORCE_32", "ON")
            .no_build_target(true)
            .build();

        println!("cargo:rustc-link-search={}/build/bin/", out_path.display());
        println!("cargo:rustc-link-lib=static=ittnotify32");
    }

    let ittnotify_bindings = bindgen::Builder::default()
        .rustfmt_bindings(true)
        .header("./include/ittnotify.h")
        .generate()
        .expect("Unable to generate bindings");

    ittnotify_bindings
        .write_to_file(out_path.join("ittnotify_bindings.rs"))
        .expect("Couldn't write bindings!");

    let jitprofiling_bindings = bindgen::Builder::default()
        .rustfmt_bindings(true)
        .header("./include/jitprofiling.h")
        .generate()
        .expect("Unable to generate bindings");

    jitprofiling_bindings
        .write_to_file(out_path.join("jitprofiling_bindings.rs"))
        .expect("Couldn't write bindings!");
}
