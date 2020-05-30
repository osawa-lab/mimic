#[derive(Debug, Clone, PartialEq, Eq, Hash)]
struct Docs {
    id: String,
    content: String,
}

fn main() {
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
