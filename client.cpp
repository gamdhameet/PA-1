/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name: Meet Gamdha
	UIN: 934003312
	Date: 16/09/2025
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include <sched.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;


int main (int argc, char *argv[]) {
	int opt;
	int p = -1;
	double t = -1.0;
	int e = -1;
	string filename = "";
	int buffer_size = MAX_MESSAGE;
	bool newchannel = false;
	
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
        switch (opt) {
            case 'p':
                p = atoi(optarg);
                break;
            case 't':
                t = atof(optarg);
                break;
            case 'e':
                e = atoi(optarg);
                break;
            case 'f':
                filename = optarg;
                break;
            case 'm':
                buffer_size = atoi(optarg);
                break;
            case 'c':
                newchannel = true;
                break;
        }
    }
	
	
	pid_t pid = fork();
	
	if (pid == 0) {
        // Child process
        string buffer_str = to_string(buffer_size);
        const char* args[] = {"./server", "-m", buffer_str.c_str(), NULL};
        execvp(args[0], (char* const*)args);
        // If execvp returns, it failed
        perror("execvp failed");
        exit(1);
    }

    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
    FIFORequestChannel* work_chan = &chan;

    if (newchannel) {
        MESSAGE_TYPE m = NEWCHANNEL_MSG;
        chan.cwrite(&m, sizeof(MESSAGE_TYPE));
        char chan_name[100];
        chan.cread(chan_name, sizeof(chan_name));
        work_chan = new FIFORequestChannel(chan_name, FIFORequestChannel::CLIENT_SIDE);
        cout << "New channel created: " << chan_name << endl;
    }
    if (p != -1 && t != -1.0 && e != -1) {
        datamsg x(p, t, e);
        work_chan->cwrite(&x, sizeof(datamsg));
        double reply;
        work_chan->cread(&reply, sizeof(double));
        cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
    }
    
    if (p != -1 && t == -1.0 && e == -1 && filename.empty()) {
        mkdir("received", 0777);
        ofstream csv_file("received/x1.csv");
        double time = 0.0;
        for (int i = 0; i < 1000; i++) {
            csv_file << time << ",";
            datamsg ecg1(p, time, 1);
            work_chan->cwrite(&ecg1, sizeof(datamsg));
            double reply1;
            work_chan->cread(&reply1, sizeof(double));
            csv_file << reply1 << ",";

            datamsg ecg2(p, time, 2);
            work_chan->cwrite(&ecg2, sizeof(datamsg));
            double reply2;
            work_chan->cread(&reply2, sizeof(double));
            csv_file << reply2 << endl;

            time += 0.004;
        }
        csv_file.close();
        cout << "Generated received/x1.csv for patient " << p << endl;
    }
    
    if (!filename.empty()) {
        mkdir("received", 0777);
        string bimdc_path = "BIMDC/" + filename;
        ifstream current_file(filename);
        if (current_file.good()) {
            current_file.close();
            string copy_command = "cp " + filename + " BIMDC/";
            system(copy_command.c_str());
        }

        // First, get the file size
        filemsg fm(0, 0);
        int len = sizeof(filemsg) + filename.size() + 1;
        char* buf = new char[len];
        memcpy(buf, &fm, sizeof(filemsg));
        strcpy(buf + sizeof(filemsg), filename.c_str());
        work_chan->cwrite(buf, len);
        int64_t file_size;
        work_chan->cread(&file_size, sizeof(int64_t));
        delete[] buf;

        string out_filepath = "received/" + filename;
        ofstream out_file(out_filepath, ios::binary);
        int64_t offset = 0;
        
        while (offset < file_size) {
            int chunk_size = min((int64_t)buffer_size, file_size - offset);
            filemsg chunk_fm(offset, chunk_size);
            

            int len = sizeof(filemsg) + filename.size() + 1;
            char* buf = new char[len];
            memcpy(buf, &chunk_fm, sizeof(filemsg));
            strcpy(buf + sizeof(filemsg), filename.c_str());
            work_chan->cwrite(buf, len);
            delete[] buf;

            char* recv_buf = new char[chunk_size];
            work_chan->cread(recv_buf, chunk_size);
            out_file.write(recv_buf, chunk_size);
            delete[] recv_buf;
            
            offset += chunk_size;
        }
        out_file.close();
        cout << "File " << filename << " downloaded to " << out_filepath << endl;
    }

	
	// example data point request
 //    char buf[MAX_MESSAGE]; // 256
 //    datamsg x(1, 0.0, 1);
	
	// memcpy(buf, &x, sizeof(datamsg));
	// chan.cwrite(buf, sizeof(datamsg)); // question
	// double reply;
	// chan.cread(&reply, sizeof(double)); //answer
	// cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	
 //    // sending a non-sense message, you need to change this
	// filemsg fm(0, 0);
	// string fname = "teslkansdlkjflasjdf.dat";
	
	// int len = sizeof(filemsg) + (fname.size() + 1);
	// char* buf2 = new char[len];
	// memcpy(buf2, &fm, sizeof(filemsg));
	// strcpy(buf2 + sizeof(filemsg), fname.c_str());
	// chan.cwrite(buf2, len);  // I want the file length;

	// delete[] buf2;
	
	// closing the channel    
	MESSAGE_TYPE m = QUIT_MSG;
    work_chan->cwrite(&m, sizeof(MESSAGE_TYPE));

    if (newchannel) {
        delete work_chan;
        chan.cwrite(&m, sizeof(MESSAGE_TYPE));
    }

    wait(NULL); // Wait for server to exit
    cout << "Client is done" << endl;

}