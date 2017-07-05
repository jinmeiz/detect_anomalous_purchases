#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include "network.h"

using namespace std;

int main(int argc, char** argv) {

  // the number of inputs is three:
  // 1st argument: batch_log.json
  // 2nd argument: stream_log.json
  // 3rd argument: flagged_purchases.json
  if (argc != 4) {
    cout << "Inputs are not correct (they shoud be batch_log.json, stream_log.json, "
        "and flagged_purchases.json)" << endl;
    return 1;
  }
  const char* fname_batch_log = argv[1];
  const char* fname_stream_log = argv[2];
  const char* fname_flagged_log = argv[3];

  ifstream in_batch_log;
  in_batch_log.open(fname_batch_log);
  if (in_batch_log.fail()) {
    std::cout << "batch_log.json opening failed\n";
    return EXIT_FAILURE;
  }
  ifstream in_stream_log;
  in_stream_log.open(fname_stream_log);
  if (in_stream_log.fail()) {
    std::cout << "stream_log.json opening failed\n";
    return EXIT_FAILURE;
  }

  ofstream out_flagged_log;
  out_flagged_log.open(fname_flagged_log);
  if (out_flagged_log.fail()) {
    std::cout << "flagged_purchases.json opening failed\n";
    return EXIT_FAILURE;
  }

  // process batch_log.json file and set the initial user network
  network user_network;
  user_network.read_batch_log(in_batch_log);
  in_batch_log.close();

  // process the stream_log.json file:
  // update user network
  // detect anomalous purchases and write them to flagged_purchases.json
  user_network.process_stream_log(in_stream_log, out_flagged_log);
  in_stream_log.close();
  out_flagged_log.close();

	puts("\nProgram successfully finished !!!");
	return EXIT_SUCCESS;
}
