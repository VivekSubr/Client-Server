use std::sync::mpsc;
use std::thread;
use std::time::Duration;
use std::sync::{Arc, Mutex};

fn main() {
    println!("Hello, world!");

    // Example 1: Basic single producer, single consumer
    basic_channel();

    // Example 2: Multiple producers, single consumer
    multiple_producers();

    // Example 3: Bounded (synchronous) channel
    bounded_channel();

    mutex_example();
}

fn basic_channel() {
    println!("1. Basic Channel (mpsc):");
    println!("{}", "-".repeat(40));

    // Create a channel - returns (Sender, Receiver)
    let (tx, rx) = mpsc::channel();
    
    // Spawn a thread that sends messages
    thread::spawn(move || { //the move keyword forces the closure to take ownership of variables it uses... here, tx
        let messages = vec![
            "Hello",
            "from",
            "the",
            "spawned",
            "thread",
        ];
        
        for msg in messages {
            tx.send(msg).unwrap(); //unwrap get values from result or option type, panics if error or none
            thread::sleep(Duration::from_millis(100));
        }
    });
    
    // Receive messages in main thread
    for received in rx {
        println!("Received: {}", received);
    }
    println!();
}

fn multiple_producers() {
    println!("2. Multiple Producers:");
    println!("{}", "-".repeat(40));
    
    let (tx, rx) = mpsc::channel();

    // Clone sender for multiple producers
    // clone usually creates a deep copy, but for channels the method returns reference to underlying channel
    let tx1 = tx.clone(); 
    let tx2 = tx.clone();

    // Producer 1
    thread::spawn(move || {
        let messages = vec!["Producer 1: Message A", "Producer 1: Message B"];
        for msg in messages {
            tx1.send(msg).unwrap();
            thread::sleep(Duration::from_millis(150));
        }
    });
    
    // Producer 2
    thread::spawn(move || {
        let messages = vec!["Producer 2: Message X", "Producer 2: Message Y"];
        for msg in messages {
            tx2.send(msg).unwrap();
            thread::sleep(Duration::from_millis(200));
        }
    });
    
    // Producer 3
    thread::spawn(move || {
        let messages = vec!["Producer 3: Data 1", "Producer 3: Data 2"];
        for msg in messages {
            tx.send(msg).unwrap();
            thread::sleep(Duration::from_millis(100));
        }
    });
    
    // Receive all messages
    // Note: We need to know when to stop, so we'll use a timeout approach
    for _ in 0..6 {
        //timout returns Result<T, E>... ie a value or error
        //match is used to handle the value and error cases here
        match rx.recv_timeout(Duration::from_secs(1)) {
            Ok(msg) => println!("Received: {}", msg),
            Err(_) => break,
        }
    }
    println!();
}

fn bounded_channel() {
    println!("3. Bounded (synchronous) Channel:");
    
    // Channel with capacity of 2
    let (tx, rx) = mpsc::sync_channel(2);

    thread::spawn(move || {
        for i in 0..5 {
            println!("Sending: {}", i);
            tx.send(i).unwrap(); // Blocks if channel is full
            println!("Sent: {}", i);
        }
    });

    thread::sleep(std::time::Duration::from_secs(1));
    
    for received in rx {
        println!("Received: {}", received);
        thread::sleep(std::time::Duration::from_millis(500));
    }

    println!();
}

fn mutex_example() {
     println!("4. Mutex Example:");
    // Arc = Atomic Reference Counting (thread-safe reference counting)
    // Mutex = Mutual Exclusion (ensures only one thread accesses data at a time)
    let counter = Arc::new(Mutex::new(0));
    let mut handles = vec![];

    for _ in 0..10 {
        let counter = Arc::clone(&counter);
        let handle = thread::spawn(move || {
            let mut num = counter.lock().unwrap();
            *num += 1;
        });
        handles.push(handle);
    }

    for handle in handles {
        handle.join().unwrap();
    }

    println!("Result: {}", *counter.lock().unwrap());
}