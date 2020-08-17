use std::env;
use std::fs::read_to_string;
use std::io::BufRead;
use std::io::BufReader;
use std::path::PathBuf;
use subprocess::{Exec, Redirection};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
struct Docs {
    id: String,
    content: String,
}

struct Evaluation {
    id: String,
    score: u32,
    compile_err: String,
}

#[derive(Clone)]
enum Output {
    Stdout(String),
    CompileErr(String),
}
use Output::*;

fn validate_stdout(stdout: String) -> u32 {
    if stdout == "answer" {
        5
    } else {
        1
    }
}

fn filename_to_id(filename: &PathBuf) -> String {
    filename
        .file_stem()
        .expect("filename is normal.")
        .to_str()
        .expect("filename is utf-8 valid")
        .to_string()
}

fn compile_run(filepath: &PathBuf, id: &str) -> Output {
    let exefilepath = filepath.with_file_name(id);
    let exefilepath = exefilepath.display();
    let filepath = filepath.display();
    let command = format!("gcc {} -o {}", filepath, exefilepath);
    let captured = Exec::shell(command)
        .stderr(Redirection::Pipe)
        .capture()
        .expect("gcc maybe exists");
    let compile_err = captured.stderr_str();
    if compile_err == "" {
        let command = format!("./{}", exefilepath);
        let captured = Exec::shell(command)
            .stdout(Redirection::Pipe)
            .capture()
            .unwrap_or_else(|_| panic!("{} should exists", exefilepath));
        let stdout = captured.stdout_str();
        Stdout(stdout)
    } else {
        CompileErr(compile_err)
    }
}

fn read_file(filename: &PathBuf) -> String {
    let mut content = match read_to_string(filename) {
        Ok(content) => content,
        Err(_) => "fail read:中身読めなかった".to_string(),
    };
    content.push_str("matsubi");
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
        let id = filename_to_id(&filename);
        let output = compile_run(&filepath, &id);
        let compile_err = match output.clone() {
            Stdout(_) => "".to_string(),
            CompileErr(err) => err,
        };
        let score = match output {
            Stdout(stdout) => validate_stdout(stdout),
            CompileErr(_) => 1u32,
        };
        evtable.push(Evaluation {
            id,
            score,
            compile_err,
        });
    }
}

#[test]
fn test() {
    let dir: PathBuf = [env!("CARGO_MANIFEST_DIR"), "tests", "data"]
        .iter()
        .collect();
    run(&dir);
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
