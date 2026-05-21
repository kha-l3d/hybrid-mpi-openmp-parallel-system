cat << 'EOF' > README.md
# Hybrid Parallel Computing System (MPI + OpenMP) 🚀

A high-performance **Hybrid Parallel System** implemented in C++ that demonstrates the integration of **MPI (Message Passing Interface)** for distributed memory management and **OpenMP (Open Multi-Processing)** for shared memory multi-threading. 

The system leverages **10 dedicated processes** ($1\text{ Master} + 9\text{ Workers}$) to perform complex, heterogeneous tasks concurrently, optimizing CPU utilization and core performance.

---

## 🏗️ System Architecture & Task Distribution

The project splits the workload across 10 independent MPI processes (Ranks 0-9). While **MPI** orchestrates the top-level communication, **OpenMP** speeds up execution internally within each process using fine-grained multi-threading.

┌──────────────────────────────────────┐
              │       MASTER PROCESS (Rank 0)        │
              └──────────────────┬───────────────────┘
                                 │
     ┌───────────────────────────┼───────────────────────────┐
     ▼                           ▼                           ▼
┌─────────────────┐         ┌─────────────────┐         ┌─────────────────┐
│ INTEGER WORKER  │         │  STRING WORKER  │         │   FILE WORKER   │
│    (Rank 1)     │         │    (Rank 2)     │         │    (Rank 3)     │
├─────────────────┤         ├─────────────────┤         ├─────────────────┤
│ Power of Four   │         │ Count Vowels    │         │ Split Odd/Even  │
│ OpenMP Reduction│         │ OpenMP Reduction│         │ OpenMP Parallel │
└─────────────────┘         └─────────────────┘         └─────────────────┘
│
▼
┌──────────────────────────┐
│  MATRIX WORKERS (4-9)    │
├──────────────────────────┤
│ Distributed Sub-blocks   │
│ Multi-threaded Addition  │
└──────────────────────────┘


### 📋 Worker Process Breakdown:
1. **Rank 0 (The Master/Manager):** Initializes the environment, generates workload parameters, distributes sub-tasks via point-to-point communication, aggregates final results, and handles display I/O.
2. **Rank 1 (Integer Compute):** Calculates the $4^{\text{th}}$ power of a given number using OpenMP parallel loops with a multiplication `reduction` clause to eliminate data races.
3. **Rank 2 (Text Parsing):** Performs vowel extraction and counting from a target string loop parallelized via OpenMP with an addition `reduction` clause.
4. **Rank 3 (File I/O Stream):** Parses a text file buffer dynamically, utilizing threads to isolate and stream odd and even lines into distinct output text files concurrently (`even_lines.txt` and `odd_lines.txt`).
5. **Ranks 4-9 (Collaborative Matrix Sub-group):** Implements **Data Parallelism** on a large matrix scale ($50 \times 50$ dimension). The Master splits rows equitably across the 6 nodes, and each node handles row-by-row matrix element addition driven by `#pragma omp parallel for`.

---

## 🛠️ Prerequisites & Installation

To compile and run this program, you need a Linux-based environment (e.g., Fedora, Ubuntu) with an MPI compiler installed.

### 1. Install OpenMPI & Development Headers
On Fedora/RHEL:

sudo dnf install openmpi openmpi-devel
On Ubuntu/Debian:

Bash
sudo apt-get install openmpi-bin libopenmpi-dev
2. Load Environment Modules (Fedora Specific)
Bash
module load mpi/openmpi-x86_64
export PATH=$PATH:/usr/lib64/openmpi/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib64/openmpi/lib
🏗️ Compilation & Execution
Compile the Code
The source must be built using an MPI C++ compiler paired with the OpenMP flag:

Bash
mpic++ -fopenmp parallel_mpi_openmp_project.cpp -o parallel_project
Grant Execution Permissions
Bash
chmod +x parallel_project
Execute the Parallel Cluster
Run the program strictly with 10 slots. If your local machine architecture contains fewer than 10 physical hardware cores, utilize the --oversubscribe flag to enforce virtual context switching:

Bash
mpirun --oversubscribe -np 10 ./parallel_project
📊 Sample Output Results
Upon successful execution, the terminal consolidates outputs directly from the Master Process:

Plaintext
[Master] Distributing jobs to worker processes...
[Rank 1] Computed Power of Four: 625
[Rank 2] Vowel Count complete. Total: 3
[Rank 3] Odd and even text lines have been isolated and written successfully.
[Matrix Group] Dynamic sub-block addition verified. Total elements calculated.

=================== FINAL RESULTS AT MASTER ===================
Numeric Computation (5^4) = 625
String Parse (Vowels in "Hello World") = 3
Matrix Addition Verification (Top 5x5 subgrid showing expected summation values):
2 2 2 2 2 
2 2 2 2 2 
2 2 2 2 2 
2 2 2 2 2 
2 2 2 2 2 
===============================================================
💡 Key Technical Concepts Covered
Distributed Memory Architecture: Isolated process spaces communicating strictly via point-to-point operations (MPI_Send / MPI_Recv) using descriptive message Tags.

Shared Memory Concurrency: Intra-process multi-threading scheduling through OpenMP pragmas.

Race Condition Prevention: Safeguarding shared variables with reduction(+:var) and reduction(*:var) structures.

Oversubscription Mapping: Forcing the process manager subsystem (PRRTE) to execute concurrent processes via virtual time-slicing slots.
EOF
