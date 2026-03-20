mod pool;

use pool::ThreadPool;
use std::{
    fs,
    io::{BufReader, prelude::*},
    net::{TcpListener, TcpStream},
};

fn main() {
    let listener = TcpListener::bind("127.0.0.1:7878").unwrap();
    let pool = ThreadPool::new(4);

    for stream in listener.incoming() {
        let stream = stream.unwrap(); //stream is of type Result<TcpStream, std::io::Error>
                                      //TcpStream is a struct around an open TCP socket

        println!("Connection established!");

        pool.execute(|| {
            handle_connection(stream);
        });
    }
}

fn handle_connection(mut stream: TcpStream) {
    let buf_reader = BufReader::new(&stream);

    /*
    **Reading from right to left:**

    1. `buf_reader.lines()` - Returns an iterator over the lines in the buffer. Each line is a `Result<String, io::Error>`.

    2. `.next()` - Gets the next item from the iterator (the first line). Returns `Option<Result<String, io::Error>>`:
       - `Some(Result<String, io::Error>)` if there's a line
       - `None` if the buffer is empty

    3. First `.unwrap()` - Unwraps the `Option`, panicking if it's `None`. After this, you have `Result<String, io::Error>`.

    4. Second `.unwrap()` - Unwraps the `Result`, panicking if there was an I/O error. After this, you have the actual `String`.
    */
    let request_line = buf_reader.lines().next().unwrap().unwrap();

    if request_line == "GET / HTTP/1.1" {
        let status_line = "HTTP/1.1 200 OK";
        let contents = fs::read_to_string("hello.html").unwrap();
        let length = contents.len();

        let response = format!(
            "{status_line}\r\nContent-Length: {length}\r\n\r\n{contents}"
        );

        stream.write_all(response.as_bytes()).unwrap();
    } else {
        let status_line = "HTTP/1.1 404 NOT FOUND";
        let contents = fs::read_to_string("404.html").unwrap();
        let length = contents.len();

        let response = format!(
            "{status_line}\r\nContent-Length: {length}\r\n\r\n{contents}"
        );

        stream.write_all(response.as_bytes()).unwrap();
    }
}