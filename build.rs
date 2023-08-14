fn main() {
    cc::Build::new()
        .cpp(true)
        .include("./include/")
        .include("./lib/")
        .file("lib/render/grayscale.cpp")
        .compile("render");
    println!("cargo:rerun-if-changed=lib/render/grayscale.cpp");
    println!("cargo:rerun-if-changed=lib/render/grayscale.hpp");
    println!("cargo:rustc-link-lib=render");
}
