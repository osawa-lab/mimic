use csv::Writer;
use serde::Serialize;
use std::env;
use std::fs::{read_to_string, File};
use std::path::PathBuf;
use subprocess::{Exec, Redirection};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
struct Docs {
    id: String,
    content: String,
}

#[derive(Debug, Serialize)]
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

struct Config {
    dir: PathBuf,
    score_output: fn(String) -> u32,
}

impl Config {
    fn new(dirname: &str, score_output: fn(String) -> u32) -> Self {
        let dir: PathBuf = [env!("CARGO_MANIFEST_DIR"), dirname].iter().collect();
        Self { dir, score_output }
    }

    fn check(self) -> Result<Self, String> {
        let dir = &self.dir;
        if !dir.exists() {
            Err(format!("{} does not exists", dir.display()))
        } else if !dir.is_dir() {
            Err(format!("{} is not a directory", dir.display()))
        } else {
            Ok(self)
        }
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
    content.push_str("");
    content
}

fn dump_csv(evtable: Vec<Evaluation>, dir: &PathBuf) {
    let csvname = dir.with_extension("csv");
    let csvfile = File::create(csvname)
        .expect("See https://doc.rust-lang.org/std/fs/struct.OpenOptions.html#errors");
    let mut wtr = Writer::from_writer(csvfile);
    for ev in evtable {
        wtr.serialize(ev).expect("add a record");
    }
}

fn score(config: Config) {
    let Config { dir, score_output } = config;
    let mut evtable = Vec::<Evaluation>::new();
    let readdir = std::fs::read_dir(&dir).unwrap_or_else(|_| {
        panic!(
            "{} ディレクトリのpermissionのせいでファイル一覧が取得できない",
            &dir.display()
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
            Stdout(stdout) => score_output(stdout),
            CompileErr(_) => 1u32,
        };
        evtable.push(Evaluation {
            id,
            score,
            compile_err,
        });
    }
    dump_csv(evtable, &dir);
}

#[test]
fn test() {
    let dir: PathBuf = [env!("CARGO_MANIFEST_DIR"), "tests", "data"]
        .iter()
        .collect();
    score(&dir);
}

fn avl_score_rule(stdout: String) -> u32 {
    if stdout == "answer" {
        5
    } else {
        1
    }
}

fn maze_score_rule(stdout: String) -> u32 {
    if stdout == "answer" {
        5
    } else {
        1
    }
}

fn main() {
    let avl = Config::new("avl", avl_score_rule);
    let maze = Config::new("maze", maze_score_rule);
    let all_config = vec![avl, maze];
    for config in all_config {
        let checked = config.check();
        match checked {
            Ok(config) => score(config),
            Err(e) => eprintln!("{:?}", e),
        }
    }
}
