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

struct OutPut {
    stdout: String,
    stderr: String,
}

struct Evaluation {
    score: u32,
}

fn validate_stdout(stdout: String) -> u32 {
    if stdout == "answer" {
        5
    } else {
        1
    }
}

fn evaluate(filename: &PathBuf, filepath: &PathBuf) -> Evaluation {
    let output = OutPut {
        stdout: "hello".to_string(),
        stderr: "".to_string(),
    };
    let OutPut { stdout, stderr } = output;
    let score = if stderr == "" {
        0
    } else {
        validate_stdout(stdout)
    };
    Evaluation { score }
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
        .expect("assume the ls command exists.");
    let br = BufReader::new(x);

    let files_under_dir: Vec<PathBuf> = br
        .lines()
        .map(|line| PathBuf::from(line.expect("lines() return Err")))
        .collect();
    let mut evtable = Vec::<Evaluation>::new();
    for filename in &files_under_dir {
        let filepath = dir.join(filename);
        let doc = read_file(&filepath);
        evtable.push(evaluate(&filename, &filepath));
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
