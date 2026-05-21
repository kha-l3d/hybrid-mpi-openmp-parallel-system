#include <mpi.h>
#include <omp.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const int MASTER = 0;
const int MATRIX_SIZE = 50;

const int TAG_INT_DATA = 10;
const int TAG_INT_RESULT = 11;

const int TAG_STRING_LEN = 20;
const int TAG_STRING_DATA = 21;
const int TAG_STRING_RESULT_LEN = 22;
const int TAG_STRING_RESULT_DATA = 23;

const int TAG_FILE_LEN = 30;
const int TAG_FILE_NAME = 31;
const int TAG_FILE_RESULT_LEN = 32;
const int TAG_FILE_RESULT_DATA = 33;

const int TAG_MATRIX_A = 40;
const int TAG_MATRIX_B = 41;
const int TAG_MATRIX_ROWS = 42;
const int TAG_MATRIX_PART = 43;

int powerOfFour(int n) {
    int ans = 1;

    #pragma omp parallel for reduction(*:ans)
    for (int i = 0; i < 4; i++) {
        ans = ans * n;
    }

    return ans;
}

bool isVowel(char c) {
    if (c == 'a' || c == 'A') return true;
    if (c == 'e' || c == 'E') return true;
    if (c == 'i' || c == 'I') return true;
    if (c == 'o' || c == 'O') return true;
    if (c == 'u' || c == 'U') return true;
    return false;
}

int countVowels(string s) {
    int c = 0;

    #pragma omp parallel for reduction(+:c)
    for (int i = 0; i < (int)s.size(); i++) {
        if (isVowel(s[i])) {
            c = c + 1;
        }
    }

    return c;
}

void splitFileLines(string fileName, string evenFile, string oddFile) {
    ifstream in(fileName.c_str());
    vector<string> lines;
    string line;

    while (getline(in, line)) {
        lines.push_back(line);
    }
    in.close();

    vector<string> evenLines((lines.size() + 1) / 2);
    vector<string> oddLines(lines.size() / 2);

    #pragma omp parallel for
    for (int i = 0; i < (int)lines.size(); i++) {
        if (i % 2 == 0) {
            evenLines[i / 2] = lines[i];
        } else {
            oddLines[i / 2] = lines[i];
        }
    }

    ofstream evenOut(evenFile.c_str());
    for (int i = 0; i < (int)evenLines.size(); i++) {
        evenOut << evenLines[i] << endl;
    }
    evenOut.close();

    ofstream oddOut(oddFile.c_str());
    for (int i = 0; i < (int)oddLines.size(); i++) {
        oddOut << oddLines[i] << endl;
    }
    oddOut.close();
}

void fillMatrices(vector<int>& a, vector<int>& b) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            a[i * MATRIX_SIZE + j] = i + j;
            b[i * MATRIX_SIZE + j] = i - j;
        }
    }
}

void printSmallPartOfMatrix(vector<int>& matrix) {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            cout << matrix[i * MATRIX_SIZE + j] << " ";
        }
        cout << endl;
    }
}

void masterWork(int size) {
    int number = 5;
    string text = "khaled Mohamed ,hamada,rami";
    string fileName = "Text File.txt";

    cout << "Master process started" << endl;
    cout << "Master sends integer to process 1" << endl;
    MPI_Send(&number, 1, MPI_INT, 1, TAG_INT_DATA, MPI_COMM_WORLD);

    cout << "Master sends string to process 2" << endl;
    int len = (int)text.size();
    MPI_Send(&len, 1, MPI_INT, 2, TAG_STRING_LEN, MPI_COMM_WORLD);
    MPI_Send(text.c_str(), len, MPI_CHAR, 2, TAG_STRING_DATA, MPI_COMM_WORLD);

    cout << "Master sends file name to process 3" << endl;
    int fileLen = (int)fileName.size();
    MPI_Send(&fileLen, 1, MPI_INT, 3, TAG_FILE_LEN, MPI_COMM_WORLD);
    MPI_Send(fileName.c_str(), fileLen, MPI_CHAR, 3, TAG_FILE_NAME, MPI_COMM_WORLD);

    vector<int> a(MATRIX_SIZE * MATRIX_SIZE);
    vector<int> b(MATRIX_SIZE * MATRIX_SIZE);
    fillMatrices(a, b);

    cout << "Master sends matrix parts to processes 4 to 9" << endl;
    int workers = 6;
    int startRow = 0;

    for (int p = 4; p <= 9; p++) {
        int rows = MATRIX_SIZE / workers;
        if (p < 4 + (MATRIX_SIZE % workers)) {
            rows = rows + 1;
        }

        MPI_Send(&rows, 1, MPI_INT, p, TAG_MATRIX_ROWS, MPI_COMM_WORLD);
        MPI_Send(&a[startRow * MATRIX_SIZE], rows * MATRIX_SIZE, MPI_INT, p, TAG_MATRIX_A, MPI_COMM_WORLD);
        MPI_Send(&b[startRow * MATRIX_SIZE], rows * MATRIX_SIZE, MPI_INT, p, TAG_MATRIX_B, MPI_COMM_WORLD);

        startRow = startRow + rows;
    }

    int intResult;
    MPI_Recv(&intResult, 1, MPI_INT, 1, TAG_INT_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int stringResultLen;
    MPI_Recv(&stringResultLen, 1, MPI_INT, 2, TAG_STRING_RESULT_LEN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    vector<char> stringBuffer(stringResultLen + 1);
    MPI_Recv(&stringBuffer[0], stringResultLen, MPI_CHAR, 2, TAG_STRING_RESULT_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    stringBuffer[stringResultLen] = '\0';

    int fileResultLen;
    MPI_Recv(&fileResultLen, 1, MPI_INT, 3, TAG_FILE_RESULT_LEN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    vector<char> fileBuffer(fileResultLen + 1);
    MPI_Recv(&fileBuffer[0], fileResultLen, MPI_CHAR, 3, TAG_FILE_RESULT_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    fileBuffer[fileResultLen] = '\0';

    vector<int> matrixResult(MATRIX_SIZE * MATRIX_SIZE);
    startRow = 0;

    for (int p = 4; p <= 9; p++) {
        int rows = MATRIX_SIZE / workers;
        if (p < 4 + (MATRIX_SIZE % workers)) {
            rows = rows + 1;
        }

        MPI_Recv(&matrixResult[startRow * MATRIX_SIZE], rows * MATRIX_SIZE, MPI_INT, p, TAG_MATRIX_PART, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        startRow = startRow + rows;
    }

    cout << endl;
    cout << "Final results at master process" << endl;
    cout << "Integer result: " << number << "^4 = " << intResult << endl;
    cout << "String result: " << stringBuffer.data() << endl;
    cout << "File result: " << fileBuffer.data() << endl;
    cout << "Matrix result: A + B, first 5x5 part:" << endl;
    printSmallPartOfMatrix(matrixResult);
}

void integerProcess() {
    int number;
    MPI_Recv(&number, 1, MPI_INT, MASTER, TAG_INT_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int result = powerOfFour(number);

    MPI_Send(&result, 1, MPI_INT, MASTER, TAG_INT_RESULT, MPI_COMM_WORLD);
}

void stringProcess() {
    int len;
    MPI_Recv(&len, 1, MPI_INT, MASTER, TAG_STRING_LEN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    vector<char> buffer(len + 1);
    MPI_Recv(&buffer[0], len, MPI_CHAR, MASTER, TAG_STRING_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    buffer[len] = '\0';

    string s = buffer.data();
    int vowels = countVowels(s);

    stringstream ss;
    ss << "Number of vowels in \"" << s << "\" = " << vowels;
    string result = ss.str();

    int resultLen = (int)result.size();
    MPI_Send(&resultLen, 1, MPI_INT, MASTER, TAG_STRING_RESULT_LEN, MPI_COMM_WORLD);
    MPI_Send(result.c_str(), resultLen, MPI_CHAR, MASTER, TAG_STRING_RESULT_DATA, MPI_COMM_WORLD);
}

void fileProcess() {
    int len;
    MPI_Recv(&len, 1, MPI_INT, MASTER, TAG_FILE_LEN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    vector<char> buffer(len + 1);
    MPI_Recv(&buffer[0], len, MPI_CHAR, MASTER, TAG_FILE_NAME, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    buffer[len] = '\0';

    string fileName = buffer.data();
    string evenFile = "even_lines.txt";
    string oddFile = "odd_lines.txt";

    splitFileLines(fileName, evenFile, oddFile);

    string result = "File was split into even_lines.txt and odd_lines.txt";
    int resultLen = (int)result.size();

    MPI_Send(&resultLen, 1, MPI_INT, MASTER, TAG_FILE_RESULT_LEN, MPI_COMM_WORLD);
    MPI_Send(result.c_str(), resultLen, MPI_CHAR, MASTER, TAG_FILE_RESULT_DATA, MPI_COMM_WORLD);
}

void matrixProcess() {
    int rows;
    MPI_Recv(&rows, 1, MPI_INT, MASTER, TAG_MATRIX_ROWS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    vector<int> a(rows * MATRIX_SIZE);
    vector<int> b(rows * MATRIX_SIZE);
    vector<int> result(rows * MATRIX_SIZE);

    MPI_Recv(&a[0], rows * MATRIX_SIZE, MPI_INT, MASTER, TAG_MATRIX_A, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&b[0], rows * MATRIX_SIZE, MPI_INT, MASTER, TAG_MATRIX_B, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    #pragma omp parallel for
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            result[i * MATRIX_SIZE + j] = a[i * MATRIX_SIZE + j] + b[i * MATRIX_SIZE + j];
        }
    }

    MPI_Send(&result[0], rows * MATRIX_SIZE, MPI_INT, MASTER, TAG_MATRIX_PART, MPI_COMM_WORLD);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank;
    int size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 10) {
        if (rank == MASTER) {
            cout << "This project must run with 10 processes." << endl;
            cout << "Use: mpirun -np 10 ./parallel_project" << endl;
            cout << "Or:  mpiexec -n 10 parallel_project.exe" << endl;
        }

        MPI_Finalize();
        return 0;
    }

    if (rank == MASTER) {
        masterWork(size);
    } else if (rank == 1) {
        integerProcess();
    } else if (rank == 2) {
        stringProcess();
    } else if (rank == 3) {
        fileProcess();
    } else {
        matrixProcess();
    }

    MPI_Finalize();
    return 0;
}
