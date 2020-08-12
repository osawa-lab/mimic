use std::env;
use std::fs::read_to_string;
use std::io::BufRead;
use std::io::BufReader;
use std::path::PathBuf;
use subprocess::Exec;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
struct Docs {
    id: String,
    content: String,
}

fn read_file(filename: &PathBuf) -> String {
    let content = match read_to_string(filename) {
        Ok(content) => content,
        Err(_) => "fail read:中身読めなかった".to_string(),
    };
    content
}

fn run(dir: PathBuf) {
    let command = format!("ls {}", dir.display());
    let x = Exec::shell(command)
        .stream_stdout()
        .expect("assume the command exists.");
    let br = BufReader::new(x);

    let files_under_dir: Vec<PathBuf> = br
        .lines()
        .map(|line| PathBuf::from(line.expect("lines() return Err")))
        .collect();
    println!("{:?}", files_under_dir);
    for filename in &files_under_dir {
        println!("{:?}", filename);
        let filepath = dir.join(filename);
        let doc = read_file(&filepath);
        dbg!(doc);
    }
}

#[test]
fn test() {
    let dir: PathBuf = [env!("CARGO_MANIFEST_DIR"), "tests", "data"]
        .iter()
        .collect();
    run(dir);
}

fn main() {
    let dir: PathBuf = [env!("CARGO_MANIFEST_DIR"), "data"].iter().collect();
    run(dir);
}
