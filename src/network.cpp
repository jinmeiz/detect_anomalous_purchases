/*
 * network.cpp
 *
 *  Created on: Jul 3, 2017
 *      Author: jinmei
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <queue>
#include <vector>
#include <algorithm>
#include <cmath>
#include "network.h"

using namespace std;
using namespace rapidjson;

// friend_degree stores the id of a user's friend
// and the degree of separation relative to the user
// e.g. a user's direct friend has a separation degree of 1
struct friend_degree {
  user_id_t friend_id;
  size_t degree;
};

// function to convert a double to a string
// and only keep two digits after decimal
string double_to_string(const double value)
{
  std::ostringstream os;
  os << fixed << setprecision(2) << value;
  return os.str();
}

// function to compare the time of two purchases
// inputs: purchase_1 - purchase information of the first purchase
//         purchase_2 - purchase information of the second purchase
// return: true if the first purchase is made earlier than the second purchase
//         false otherwise
bool comp_purchase_time(const purchase_info& purchase_1,
    const purchase_info& purchase_2) {

//  // If purchases do not come in time order,
//  // unblock the following commented codes
//  // get the purchase times
//  uint64_t purchase1_time = purchase_1.tm_info.purchase_time;
//  uint64_t purchase2_time = purchase_2.tm_info.purchase_time;
//
//  if(purchase1_time < purchase2_time)
//    return true;
//  if(purchase1_time > purchase2_time)
//    return false;

  // check purchase order
  size_t purchase1_order = purchase_1.tm_info.purchase_order;
  size_t purchase2_order = purchase_2.tm_info.purchase_order;
  return purchase1_order < purchase2_order;
}

const std::unordered_map<user_id_t, user_info>& network::get_network() {
  return map_users_;
}

void network::process_batch_entries(const Document& entry) {

  string event_type = entry["event_type"].GetString();
  string timestamp = entry["timestamp"].GetString();

  if (!event_type.compare("purchase")) {
    // this is a purchase event
    // increase purchase order by one
    const size_t purchase_order = ++purchase_order_;

    user_id_t id = stoi(entry["id"].GetString());
    user_info& curr_user = map_users_[id];

    double amount = stod(entry["amount"].GetString());

    // update purchases
    curr_user.update_purchases(timestamp, purchase_order, amount, T_);
  }
  else if (!event_type.compare("befriend")
      || !event_type.compare("unfriend")) {
    // this is a befriend or unfriend event

    user_id_t id1 = stoi(entry["id1"].GetString());
    user_id_t id2 = stoi(entry["id2"].GetString());

    user_info& curr_user1 = map_users_[id1];
    user_info& curr_user2 = map_users_[id2];

    if (id1 != id2) {
      if (!event_type.compare("befriend")) {
        curr_user1.add_friend(id2);
        curr_user2.add_friend(id1);
      } else {
        curr_user1.remove_friend(id2);
        curr_user2.remove_friend(id1);
      }
    } else {
        cerr << "Error: befriend or unfriend event for same user "
            << id1 << endl;
    }
  } else {
      cerr << "Error: can not recognize this event type: "
          << event_type << endl;
  }
}

void network::read_batch_log(ifstream& in_batch_log) {

  string line;
  while (getline(in_batch_log, line)) {
    // skip empty lines
    if (line.empty())
      continue;

    Document doc;
    doc.Parse(line.c_str());
    if(!doc.IsObject()) {
      cerr << "Error: doc is not object" << endl;
      continue;
    }

    if (doc.HasMember("D")) {
      // read and D & T
      D_ = stoi(doc["D"].GetString());
      T_ = stoi(doc["T"].GetString());

    } else if(doc.HasMember("event_type")) {
      // process different events
      process_batch_entries(doc);
    } else {
      cerr << "Error: event_type is present in this line" << endl;
    }
  }
}

unordered_set<user_id_t> network::get_friends_network(const user_id_t user_id) {
  // obtain user map
  const unordered_map<size_t, user_info>& map_users = get_network();

  // firends_in_network contains id's of all friends in the network
  unordered_set<user_id_t> friends_in_network;
  // create a queue to store the friend ids visited so far
  queue<friend_degree> q_process_friends;
  q_process_friends.push({user_id, 0});

  while (!q_process_friends.empty()) {
    friend_degree curr_friend_degree = q_process_friends.front();
    q_process_friends.pop();

    user_id_t curr_friend_id = curr_friend_degree.friend_id;
    size_t curr_degree = curr_friend_degree.degree + 1;

    const auto iter_curr_friend = map_users.find(curr_friend_id);
    // if this friend exist, continue with the following process
    if (iter_curr_friend != map_users.end()) {
      const user_info& curr_friend = iter_curr_friend->second;
      const unordered_set<user_id_t>& curr_user_friends = curr_friend.get_friend_list();

      // at the highest separation level, skip those friends's friends
      if (curr_degree == D_ + 1) {
        continue;
      }

      // traverse friends's friends
      for (const auto& friend_id : curr_user_friends) {
        // if a friend is already in the list, skip this friend
        if (friends_in_network.find(friend_id) != friends_in_network.end())
          continue;

        // push this friend to the queue
        q_process_friends.push({friend_id, curr_degree});

        friends_in_network.insert(friend_id);
      }
    } else
      cerr << "Error: can not find friend with id " << curr_friend_id << endl;
  }

  // remove user from the friend list
  friends_in_network.erase(user_id);

  return friends_in_network;
}

vector<purchase_info> network::friend_purchases(
    const unordered_set<user_id_t>& firends_in_network) {

  // create a vector for storing all the most recent T purchase
  vector<purchase_info> purchases;
  purchases.reserve(T_*2);

  // obtain user map
  const unordered_map<size_t, user_info>& map_users = get_network();
  
  // obtain the first friend in the network
  const user_id_t first_friend_id = *firends_in_network.begin();
  const auto iter_first_friend = map_users.find(first_friend_id);

  // check if this friend exist
  if (iter_first_friend != map_users.end()) {
    const user_info& first_friend = iter_first_friend->second;
    const deque<purchase_info>& first_friend_purchases = first_friend.get_purchase_record();

    // copy the first friend's purchase to purchases
    if (!first_friend_purchases.empty())
      purchases.insert(purchases.end(),
          first_friend_purchases.begin(), first_friend_purchases.end());
  } else
    cerr << "Error: can not find the first friend with id " << first_friend_id << endl;

  for (auto iter_friend = ++firends_in_network.begin();
      iter_friend != firends_in_network.end(); ++iter_friend) {

    const user_id_t friend_id = *iter_friend;
    const auto iter_this_friend = map_users.find(friend_id);

    // if this friend exist, continue with the following process
    if (iter_this_friend != map_users.end()) {

      const user_info& curr_friend = iter_this_friend->second;
      const deque<purchase_info>& friend_purchases = curr_friend.get_purchase_record();

      // copy current friend's purchases into the second half of T_purchases container
      if (!friend_purchases.empty()) {

        // resize purchases to be able to hold purchases of previous and current friends
        const size_t size_friend_purchases = friend_purchases.size();
        const size_t size_purchase_merge = purchases.size() + size_friend_purchases;
        purchases.resize(size_purchase_merge);

        // merge and sort purchases of previous and current friends
        // using reverse iterators
        merge(purchases.rbegin() + size_friend_purchases, purchases.rend() ,
            friend_purchases.rbegin(), friend_purchases.rend(),
            purchases.rbegin(), comp_purchase_time);

        // resize purchases
        purchases.resize(std::min(T_, purchases.size()));
      }
    } else
      cerr << "Error: can not find friend with id " << friend_id << endl;
  }

  return purchases;
}

bool network::compute_mean_sd(const user_id_t user_id,
    double& mean, double& standard_deviation) {

  // obtain the user's friends in the D-degree social network
  const unordered_set<user_id_t> friends_in_network = get_friends_network(user_id);

  // obtain the last T purchases in the social network
  const vector<purchase_info> friends_purchases = friend_purchases(friends_in_network);

  size_t n_purchases = friends_purchases.size();
  // if the number of purchases is larger than 1,
  // compute the standard deviation and mean of the last T purchases
  if (n_purchases > 1) {

    double sum_purchases = 0.0;
    double sum2_purchases = 0.0;
    for (const auto& purchase_info: friends_purchases) {
      const double purchase_amount = purchase_info.amount;
      sum_purchases += purchase_amount;
      sum2_purchases += purchase_amount * purchase_amount;
    }

    // compute the mean and standard deviation of T purchases
    const double mean_purchases = sum_purchases / n_purchases;
    mean = mean_purchases;

    const double variance_purchases = sum2_purchases/n_purchases
        - mean_purchases * mean_purchases;
    const double sd_purchases = sqrt(variance_purchases);
    standard_deviation = sd_purchases;

    return true;
  }

  return false;
}

bool network::process_stream_entries(const Document& entry,
    double& mean, double& standard_deviation) {

  string event_type = entry["event_type"].GetString();
  string timestamp = entry["timestamp"].GetString();

  if (!event_type.compare("purchase")) {
    // this is a purchase event

    // create or find the user
    user_id_t id = stoi(entry["id"].GetString());
    user_info& curr_user = map_users_[id];

    // obtain purchase amount
    double amount = stod(entry["amount"].GetString());

    // update purchase order
    const size_t purchase_order = ++purchase_order_;

    // update the user's purchases
    curr_user.update_purchases(timestamp, purchase_order, amount, T_);

    // if the user has friends,
    // proceed to check if this purchase is anomalous
    if (!curr_user.get_friend_list().empty()) {

      // check if there is enough purchase history (>= 2 purchases) in a user's network
      // if true, calculate the mean and standard deviation of recent T purchases in the network
      const bool enough_purchase_history = compute_mean_sd(id, mean, standard_deviation);

      // with enough purchase history, check if the purchase is anomalous
      // that is the amount of this purchase is larger than 3 standard deviations plus the mean
      if (enough_purchase_history) {
        if (amount > (mean + standard_deviation * 3))
          return true;
      }
    }
  } else if (!event_type.compare("befriend")
      || !event_type.compare("unfriend")) {
    // this is a befriend or unfriend event

    // create or find users
    user_id_t id1 = stoi(entry["id1"].GetString());
    user_id_t id2 = stoi(entry["id2"].GetString());
    user_info& curr_user1 = map_users_[id1];
    user_info& curr_user2 = map_users_[id2];

    if (id1 != id2) {
      if (!event_type.compare("befriend")) {
        curr_user1.add_friend(id2);
        curr_user2.add_friend(id1);
      } else {
        curr_user1.remove_friend(id2);
        curr_user2.remove_friend(id1);
      }
    } else
      cerr << "Error: befriend or unfriend event for same user "
          << id1 << endl;
  } else {
      cerr << "Error: can not recognize this event type: "
          << event_type << endl;
  }

  return false;
}

void network::process_stream_log(ifstream& in_stream_log, ofstream& out_flagged_log) {

  string line;
  while (getline(in_stream_log, line)) {
    // skip empty lines
    if (line.empty())
      continue;

    Document doc;
    doc.Parse(line.c_str());
    if(!doc.IsObject()) {
      cerr << "Error: doc is not object" << endl;
      continue;
    }

    if(doc.HasMember("event_type")) {
      // process different events and flag any anomalous purchase,
      // if true, compute the mean and standard deviation
      double mean, standard_deviation;
      const bool flagged_purchase = process_stream_entries(doc, mean, standard_deviation);

      // write anomalous purchases to a output file
      if (flagged_purchase) {
        const string str_mean = double_to_string(mean);
        const string str_sd = double_to_string(standard_deviation);
        line.erase(line.end()-1);
        const string new_line = line + ", \"mean\": \"" + str_mean
            + "\", \"sd\": \"" + str_sd + "\"}";
        out_flagged_log << new_line << endl;
      }
    } else {
      cerr << "Error: event_type is present in this line" << endl;
    }
  }

}
