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
    let command = format!("gcc {}", filepath.display());
    let x = Exec::shell(command)
        .stream_stdout()
        .expect("gcc maybe exists");
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

fn run(dir: &PathBuf) {
    let mut evtable = Vec::<Evaluation>::new();
    let readdir = std::fs::read_dir(dir).unwrap_or_else(|_| {
        panic!(
            "{} ディレクトリのpermissionのせいでファイル一覧が取得できない",
            dir.display()
        )
    });
    for entry in readdir {
        let filename = entry.expect("多分大丈夫").path();
        let filepath = dir.join(&filename);
        let doc = read_file(&filepath);
        let evaluation = evaluate(&filename, &filepath);
        evtable.push(evaluation);
    }
}

#[test]
fn test() {
    let dir: PathBuf = [env!("CARGO_MANIFEST_DIR"), "tests", "data"]
        .iter()
        .collect();
    run(dir);
}

fn check(dir: &PathBuf) -> Result<&PathBuf, String> {
    if !dir.exists() {
        Err(format!("{} does not exists", dir.display()))
    } else if !dir.is_dir() {
        Err(format!("{} is not a directory", dir.display()))
    } else {
        Ok(dir)
    }
}

fn main() {
    let avl_dir: PathBuf = [env!("CARGO_MANIFEST_DIR"), "avl"].iter().collect();
    let maze_dir: PathBuf = [env!("CARGO_MANIFEST_DIR"), "maze"].iter().collect();
    let all_dirs = vec![avl_dir, maze_dir];
    for checked in all_dirs.iter().map(|d| check(d)) {
        match checked {
            Ok(dir) => run(dir),
            Err(e) => eprintln!("{:?}", e),
        }
    }
}
