use csv::Writer;
use regex::Regex;
use serde::Serialize;
use std::env;
use std::{
    convert::TryInto,
    fs::{read_to_string, File},
    path::{Display, PathBuf},
};
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
        .expect("filename is normal.")
        .to_str()
        .expect("filename is utf-8 valid")
        .to_string()
}

fn compile_run(filepath: &PathBuf, id: &str, run: fn(Display) -> Vec<String>) -> Output {
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

fn score(config: Config) {
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
        let filename = entry.expect("多分大丈夫").path();
        let filepath = dir.join(&filename);
        let _doc = read_file(&filepath);
        let id = filename_to_id(&filename);
        let output = compile_run(&filepath, &id, run);
        let compile_err = match output.clone() {
            Stdouts(_) => "".to_string(),
            CompileErr(err) => err,
        };
        let score = match output {
            Stdouts(stdout) => score_output(stdout),
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

fn exec_shell(command: String) -> String {
    let captured = Exec::shell(command)
        .stdout(Redirection::Pipe)
        .capture()
        .expect("command shoulf exists");
    captured.stdout_str()
}

fn avl_run(exefilepath: Display) -> Vec<String> {
    let command = format!("./{}", exefilepath);
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
    let answers: Vec<u32> = vec![1, 2, 3, 4, 5];
    for (p_num, &correct_answer) in answers.iter().enumerate() {
        if let Some(student_answer) =
            avl_extract_answer((p_num + 1).try_into().unwrap(), &stdout[0])
        {
            if correct_answer == student_answer {
                score += 1;
            }
        }
    }
    score
}

#[test]
fn test_avl_score_rule() {
    let input = r###"
ans1=1\n
ans2=2\n
ans3=3\n
ans4=4\n
ans5=5\n"###
        .to_string();
    let score = avl_score_rule(vec![input]);
    assert_eq!(score, 5);
}

fn maze_run(exefilepath: Display) -> Vec<String> {
    let mut stdouts = Vec::<String>::new();
    let args = vec![("a", "maze_data_queue1.txt"), ("b", "maze_data_stack1.txt")];
    for (maze_type, filename) in args {
        let command = format!("echo {} {} |./{}", maze_type, filename, exefilepath);
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
    score += if stdout[0].contains(&answer_a) { 5 } else { 1 };
    score += if stdout[1].contains(&answer_b) { 5 } else { 1 };
    score
}

fn main() {
    let avl = Config::new("avl", avl_run, avl_score_rule);
    let maze = Config::new("maze", maze_run, maze_score_rule);
    let all_config = vec![avl, maze];
    for config in all_config {
        let checked = config.check();
        match checked {
            Ok(config) => score(config),
            Err(e) => eprintln!("{:?}", e),
        }
    }
}
