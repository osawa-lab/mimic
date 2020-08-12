use std::env;
use std::fs::read_to_string;
use std::io::BufRead;
use std::io::BufReader;
use std::path::PathBuf;
use strsim::levenshtein;
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

fn main() {
    let dir: PathBuf = [env!("CARGO_MANIFEST_DIR"), "data"].iter().collect();
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

    let s1 = "Hello, world!".to_string();
    let s2 = "Hello, world!".to_string();
    let s3 = "goodnight, world!".to_string();
    let input_docs = vec![s1, s2, s3];
    let cluster = make_cluster(&input_docs);
    println!("{:?}", cluster);
}

fn make_cluster(docs: &Vec<String>) -> Vec<Docs> {
    let d = levenshtein(&docs[1], &docs[2]);
    dbg!(d);
    vec![Docs {
        id: "s1234".to_string(),
        content: docs[0].clone(),
    }]
}
