use clap::Parser;
use dam::{
    context_tools::*,
    dam_macros::context_macro,
    simulation::{InitializationOptionsBuilder, ProgramBuilder, RunMode, RunOptionsBuilder},
    utility_contexts::{ConsumerContext, GeneratorContext},
};

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("/home/nzhang/sst-dam-benchmarks/common/workloads.h");

        fn compute_fibonacci(value: u64) -> u64;
    }
}

#[derive(Parser, Debug)]
struct Args {
    #[arg(long)]
    fib_size: u64,

    #[arg(long)]
    iters: u64,

    #[arg(long)]
    depth: usize,

    #[arg(long)]
    num_trees: usize,

    #[arg(long)]
    latency: u64,

    #[arg(long)]
    channel_depth: Option<usize>,

    #[arg(long)]
    opt: bool,

    #[arg(long)]
    fifo_mode: bool,

    #[arg(long, default_value_t = 0)]
    imbalance: u64,
}

#[context_macro]
struct SumWithFibContext {
    a: Receiver<u64>,
    b: Receiver<u64>,
    out: Sender<u64>,
    fib_factor: u64,
}

impl Context for SumWithFibContext {
    fn run(&mut self) {
        loop {
            match (self.a.dequeue(&self.time), self.b.dequeue(&self.time)) {
                (Ok(ce1), Ok(ce2)) => self
                    .out
                    .enqueue(
                        &self.time,
                        ChannelElement {
                            time: self.time.tick() + 1,
                            data: ce1.data + ce2.data + ffi::compute_fibonacci(self.fib_factor),
                        },
                    )
                    .unwrap(),
                (Err(_), Err(_)) => return,
                _ => panic!("Mismatched A and B streams"),
            }
        }
    }
}

impl SumWithFibContext {
    fn new(a: Receiver<u64>, b: Receiver<u64>, out: Sender<u64>, fib_factor: u64) -> Self {
        let s = Self {
            a,
            b,
            out,
            fib_factor,
            context_info: Default::default(),
        };

        s.a.attach_receiver(&s);
        s.b.attach_receiver(&s);
        s.out.attach_sender(&s);
        s
    }
}

fn main() {
    let args = Args::parse();

    let mut program = ProgramBuilder::default();
    let mk_channel = |pb: &mut ProgramBuilder| match args.channel_depth {
        Some(d) => pb.bounded_with_latency(d, args.latency, 1),
        None => pb.unbounded_with_latency(args.latency, 1),
    };

    for i in 0..args.num_trees {
        let mut inputs = vec![];
        let fib_size = if i == 0 {
            args.imbalance + args.fib_size
        } else {
            args.fib_size
        };

        // populate previous layer with producers
        inputs.resize_with(1 << args.depth, || {
            let (snd, rcv) = mk_channel(&mut program);
            program.add_child(GeneratorContext::new(|| 0..args.iters, snd));
            rcv
        });

        while inputs.len() > 1 {
            let left = inputs.pop().unwrap();
            let right = inputs.pop().unwrap();
            let (out, out_rcv) = mk_channel(&mut program);

            program.add_child(SumWithFibContext::new(left, right, out, fib_size));

            inputs.push(out_rcv);
        }

        program.add_child(ConsumerContext::new(inputs.pop().unwrap()));
    }

    let start = std::time::Instant::now();
    let initialized = program
        .initialize(
            InitializationOptionsBuilder::default()
                .run_flavor_inference(args.opt)
                .build()
                .unwrap(),
        )
        .unwrap();
    let init_time = start.elapsed();
    println!("Setup Time: {:?}", init_time);
    let exec = initialized.run(
        RunOptionsBuilder::default()
            .mode(if args.fifo_mode {
                RunMode::FIFO
            } else {
                RunMode::Simple
            })
            .build()
            .unwrap(),
    );
    println!("Elapsed: {:?}", exec.elapsed_cycles());
}
