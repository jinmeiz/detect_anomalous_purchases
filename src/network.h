/*
 * network.h
 *
 *  Created on: Jul 3, 2017
 *      Author: jinmei
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#include <unordered_map>
#include "user_info.h"
#include "include/rapidjson/document.h"

// network class maintains the user network and purchase history
class network {
  private:
    // number of degrees in social network
    std::size_t D_{};
    // number of recent purchases in a user's network
    std::size_t T_{};
    // unordered map containing all user information
    std::unordered_map<user_id_t, user_info> map_users_{};
    // the order of a purchase when it is read
    std::size_t purchase_order_ = 0;

    // function to obtain the user network
    // return: an unordered_map containing all users' ids and information
    const std::unordered_map<user_id_t, user_info>& get_network();

    // function to process a line in the file stream for batch_log.json:
    //        add a purchase, add a friend, or delete a friend for a user
    // input: entry - rapidjson object containing an event and its information
    void process_batch_entries(const rapidjson::Document& entry);

    // function to obtain ids of all friends in a user's social network
    //         (D degree of separation)
    // input:  user_id - a user id
    // return: an unordered set containing the friend ids of a user within D degree of separation
    std::unordered_set<user_id_t> get_friends_network(const user_id_t user_id);

    // function to obtain the recent T purchases in a user's network
    // input:  firends_in_network - friend ids of a user within D degree of separation
    // return: a vector of purchases
    std::vector<purchase_info> friend_purchases(const std::unordered_set<user_id_t>&
        firends_in_network) ;

    // function to compute mean and standard deviations of the last T purchases
    //          within the user's D degree social network
    // input:   user_id - a user id
    // outputs: mean - reference to mean of recent T purchases in the user's network
    //          standard_deviation - reference to standard deviation of T recent purchases
    // return: true if mean and standard deviation are calculated
    //         false if there are less than 2 purchases in the network
    bool compute_mean_sd(const user_id_t user_id, double& mean, double& standard_deviation);

    // function to process a line in the file stream for stream_log.json:
    //          process a purchase, add a friend, or remove a friend
    // inputs:  entry - rapidjson object containing an event and its information
    // outputs: mean - reference to mean of recent T purchases in the user's network
    //          standard_deviation - reference to standard deviation of T recent purchases
    // return:  true if a purchase is anomalous (it is larger than mean+3*standard_deviation)
    //          false otherwise
    bool process_stream_entries(const rapidjson::Document& entry,
        double& mean, double& standard_deviation);

  public:
    network() = default;

    // function to read batch_log.json file to
    //        obtain degree of separation and maximum purchase history
    //        and set the initial state for user network and purchase history
    // input: in_batch_log - input file stream for batch_log.json
    void read_batch_log(std::ifstream& in_batch_log);

    // function to read stream_log.json file for
    //         updating user network and purchase history,
    //         detecting anomalous purchases, and
    //         writing those purchases along with associated means and standard deviations to a file
    // input:  in_stream_log - input file stream for stream_log.json
    // output: out_flagged_log - output file stream for flagged_purchases.json
    void process_stream_log(std::ifstream& in_stream_log, std::ofstream& out_flagged_log);
};




#endif /* NETWORK_H_ */
