use csv::Writer;
use regex::Regex;
use serde::Serialize;
use std::env;
use std::{
    convert::TryInto,
    ffi::OsStr,
    fs::{read_to_string, File},
    path::{Display, PathBuf},
};
use subprocess::{Exec, Redirection};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
struct Docs {
    id: String,
    content: String,
}

#[derive(Debug, Eq, PartialEq, Serialize)]
struct Evaluation {
    id: String,
    score: u32,
    compile_err: String,
}

#[derive(Clone)]
enum Output {
    Stdouts(Vec<String>),
    CompileErr(String),
}
use Output::*;

struct Config {
    dir: PathBuf,
    run: fn(Display) -> Vec<String>,
    score_output: fn(Vec<String>) -> u32,
}

impl Config {
    fn new(
        dirname: &str,
        run: fn(Display) -> Vec<String>,
        score_output: fn(Vec<String>) -> u32,
    ) -> Self {
        let dir: PathBuf = [env!("CARGO_MANIFEST_DIR"), dirname].iter().collect();
        Self {
            dir,
            run,
            score_output,
        }
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
        .expect("this file must exist.")
        .to_str()
        .expect("filename is utf-8 valid")
        .to_string()
}

fn compile_run(filepath: &PathBuf, exefilename: &str, run: fn(Display) -> Vec<String>) -> Output {
    let exefilepath = filepath.with_file_name(exefilename);
    let exefilepath = exefilepath.display();
    let filepath = filepath.display();
    let command = format!("gcc {} -o {}", filepath, exefilepath);
    let captured = Exec::shell(command)
        .stderr(Redirection::Pipe)
        .capture()
        .expect("gcc maybe exists");
    let compile_err = captured.stderr_str();
    if compile_err == "" {
        let stdouts = run(exefilepath);
        Stdouts(stdouts)
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

fn score(config: Config) -> (Vec<Evaluation>, PathBuf) {
    let Config {
        dir,
        run,
        score_output,
    } = config;
    let mut evtable = Vec::<Evaluation>::new();
    let readdir = std::fs::read_dir(&dir).unwrap_or_else(|_| {
        panic!(
            "{} ディレクトリのpermissionのせいでファイル一覧が取得できない",
            &dir.display()
        )
    });
    for entry in readdir {
        let source_fullpath = entry.expect("多分大丈夫").path();
        assert!(source_fullpath.is_absolute());
        if source_fullpath.extension() != Some(OsStr::new("c")) {
            continue;
        }
        let _doc = read_file(&source_fullpath);
        let id = filename_to_id(&source_fullpath);
        let exefilename = format!("{}.out", &id);
        let output = compile_run(&source_fullpath, &exefilename, run);
        let compile_err = match output.clone() {
            Stdouts(_) => String::new(),
            CompileErr(err) => err,
        };
        let score = match output {
            Stdouts(stdout) => score_output(stdout),
            CompileErr(_) => 0u32,
        };
        evtable.push(Evaluation {
            id,
            score,
            compile_err,
        });
    }
    (evtable, dir)
}

fn exec_shell(command: String) -> String {
    let captured = Exec::shell(command)
        .stdout(Redirection::Pipe)
        .capture()
        .expect("command shoulf exists");
    captured.stdout_str()
}

fn avl_run(exefilepath: Display) -> Vec<String> {
    let command = format!("{}", exefilepath);
    vec![exec_shell(command)]
}

fn avl_extract_answer(p_num: u32, input: &str) -> Option<u32> {
    let numstr = p_num.to_string();
    let re = Regex::new(&format!("ans{}=(\\d+)", numstr)).unwrap();
    match re.captures(&input) {
        None => None,
        Some(cap) => {
            let dstr = cap.get(1).unwrap().as_str();
            Some(dstr.parse::<u32>().unwrap())
        }
    }
}

#[test]
fn test_avl_extract_answer() {
    let input = "ans1=12".to_string();
    let parsed = avl_extract_answer(1, &input);
    assert_eq!(parsed, Some(12u32));
}

fn avl_score_rule(stdout: Vec<String>) -> u32 {
    assert_eq!(stdout.len(), 1);
    let mut score = 0;
    let answers: Vec<u32> = vec![64, 6, 50, 88, 1];
    for (p_num, &correct_answer) in answers.iter().enumerate() {
        if let Some(student_answer) =
            avl_extract_answer((p_num + 1).try_into().unwrap(), &stdout[0])
        {
            if correct_answer == student_answer {
                score += 4;
            }
        }
    }
    score
}

#[test]
fn test_avl_score_rule() {
    let input = r###"
ans1=64\n
ans2=6\n
ans3=50\n
ans4=88\n
ans5=1\n"###
        .to_string();
    let score = avl_score_rule(vec![input]);
    assert_eq!(score, 20);
}

fn maze_run(exefilepath: Display) -> Vec<String> {
    let mut stdouts = Vec::<String>::new();
    let args = vec![
        ("a", PathBuf::from("maze_data_queue1.txt")),
        ("b", PathBuf::from("maze_data_stack1.txt")),
    ];
    for (maze_type, filename) in args {
        assert!(filename.exists());
        let filename = filename.display();
        let command = format!("echo {} {} |{}", maze_type, filename, exefilepath);
        let stdout = exec_shell(command);
        stdouts.push(stdout)
    }
    stdouts
}

fn maze_score_rule(stdout: Vec<String>) -> u32 {
    let answer_a = PathBuf::from("maze_a_answer.txt");
    assert!(answer_a.exists());
    let answer_b = PathBuf::from("maze_b_answer.txt");
    assert!(answer_b.exists());
    let answer_a = read_to_string(answer_a).expect("exists but fail to read");
    let answer_b = read_to_string(answer_b).expect("exists but fail to read");

    assert_eq!(stdout.len(), 2);
    let mut score: u32 = 0;
    score += if stdout[0].contains(&answer_a) { 10 } else { 0 };
    score += if stdout[1].contains(&answer_b) { 10 } else { 0 };
    score
}

#[test]
fn test_avl_score() {
    let avl = Config::new("tests/avl", avl_run, avl_score_rule);
    let checked = avl.check();
    assert!(checked.is_ok());
    let (evtable, _dir) = score(checked.unwrap());
    let expected_evtable = vec![Evaluation {
        id: "avl".to_string(),
        score: 20,
        compile_err: String::new(),
    }];
    assert_eq!(expected_evtable, evtable);
}

#[test]
fn test_maze_score() {
    let maze = Config::new("tests/maze", maze_run, maze_score_rule);
    let checked = maze.check();
    assert!(checked.is_ok());
    let (evtable, _dir) = score(checked.unwrap());
    let expected_evtable = vec![Evaluation {
        id: "maze".to_string(),
        score: 20,
        compile_err: String::new(),
    }];
    assert_eq!(expected_evtable, evtable);
}

fn main() {
    let avl = Config::new("avl", avl_run, avl_score_rule);
    let maze = Config::new("maze", maze_run, maze_score_rule);
    let all_config = vec![avl, maze];
    for config in all_config {
        let checked = config.check();
        match checked {
            Ok(config) => {
                let (evtable, dir) = score(config);
                dump_csv(evtable, &dir);
            }
            Err(e) => eprintln!("{:?}", e),
        }
    }
}
