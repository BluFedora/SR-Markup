//
// Shareef Abdoul-Raheem
//

// [https://michael-f-bryan.github.io/rust-ffi-guide/cbindgen.html]

extern crate cbindgen;

use cbindgen::Config;

use std::env;
use std::path::PathBuf;

fn main() {
  let package_name = env::var("CARGO_PKG_NAME").unwrap();
  let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
  let output_file = target_dir()
    .join(format!("{}.h", package_name))
    .display()
    .to_string();

  let cbindgen_config = Config::from_root_or_default(&crate_dir);
  
  cbindgen::Builder::new()
    .with_crate(crate_dir)
    .with_config(cbindgen_config)
    .generate()
    .expect("Unable to generate bindings")
    .write_to_file(&output_file);
}

// Find the location of the `target/` directory. Note that this may be
// overridden by `cmake`, so we also need to check the `CARGO_TARGET_DIR` variable.
fn target_dir() -> PathBuf {
  if let Ok(target) = env::var("CARGO_TARGET_DIR") {
    PathBuf::from(target)
  } else {
    PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap()).join("target")
  }
}
