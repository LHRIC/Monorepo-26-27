use std::env::var;
use std::fs::read_to_string;
use std::path::PathBuf;

use dbc_codegen::Config;

const DBC_PATH: &str = "../12.00_DBC/LHRDB.dbc";

fn main() {
    let dbc_file = read_to_string(DBC_PATH).unwrap();
    println!("cargo:rerun-if-changed={DBC_PATH}");

    let path = PathBuf::from(var("OUT_DIR").unwrap()).join("messages.rs");
    Config::builder()
        .dbc_name("lhrdb.dbc")
        .dbc_content(&dbc_file)
        .check_ranges(dbc_codegen::FeatureConfig::Never)
        .build()
        .write_to_file(path)
        .unwrap();
}
