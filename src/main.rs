// use std::fs::File;
use std::env;
use std::io::BufRead;
use std::io::BufReader;
use std::path::PathBuf;
use subprocess::Exec;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
struct Docs {
    id: String,
    content: String,
}

fn main() {
    let dir: PathBuf = [env!("CARGO_MANIFEST_DIR"), "data"].iter().collect();
    let command = format!("ls {}", dir.display());
    let x = Exec::cmd(command)
        .stream_stdout()
        .expect("assume the command exists.");
    let br = BufReader::new(x);
    // for (i, line) in br.lines().enumerate() {
    //     println!("{}: {}", i, line.unwrap());
    // }

    let files_under_dir: Vec<_> = br.lines().iter();
    // let file = File::open(filename).unwrap();
    // let reader = BufReader::new(&file);

    let s1 = "Hello, world!".to_string();
    let s2 = "Hello, world!".to_string();
    let s3 = "goodnight, world!".to_string();
    let input_docs = vec![s1, s2, s3];
    let cluster = make_cluster(&input_docs);
    println!("{:?}", cluster);
}

fn make_cluster(docs: &Vec<String>) -> Vec<Docs> {
    vec![Docs {
        id: "s1234".to_string(),
        content: docs[0].clone(),
    }]
}
