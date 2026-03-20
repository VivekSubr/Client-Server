use std::{sync::{mpsc, Arc, Mutex}, thread};

pub struct ThreadPool {
    threads: Vec<thread::JoinHandle<()>>,
    workers: Vec<Worker>,
    sender: mpsc::Sender<Job>,
}

type Job = Box<dyn FnOnce() + Send + 'static>;

struct Worker {
    id: usize,
    thread: thread::JoinHandle<()>,
}

impl ThreadPool {
    //'new' is the constructor
    pub fn new(size: usize) -> ThreadPool {
        assert!(size > 0);

        let (sender, receiver) = mpsc::channel();
        let receiver = Arc::new(Mutex::new(receiver));

        let mut workers = Vec::with_capacity(size);
        for id in 0..size {
            //the worker new takes id and clone of reciever
            workers.push(Worker::new(id, Arc::clone(&receiver)));
        }

        ThreadPool {
            threads: Vec::with_capacity(size),
            workers,
            sender,
        }
    }

    pub fn execute<F>(&self, f: F)
    where
        F: FnOnce() + Send + 'static, //these are traits for closure 'F'
    {
        //Box is a smart pointer that allocates on heap, and owns it's data.
        let job = Box::new(f); 
        self.sender.send(job).unwrap(); //send job 'f' to channel
    }
}

impl Worker {
    fn new(id: usize, receiver: Arc<Mutex<mpsc::Receiver<Job>>>) -> Worker {
        let thread = thread::spawn(move || loop {
            /*
                this is a tricky statement, 
            */
            let job = receiver.lock().unwrap().recv().unwrap();
            println!("Worker {id} got a job; executing.");
            job();
        });

        Worker { id, thread }
    }
}