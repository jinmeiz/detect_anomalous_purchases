/*
 * user_info.h
 *
 *  Created on: Jun 28, 2017
 *      Author: jinmei
 */

#include <string>
#include <unordered_map>
#include <iterator>
#include <deque>
#include <unordered_set>
#include <ctime>
#include "include/rapidjson/document.h"

#ifndef USER_INFO_H_
#define USER_INFO_H_

typedef std::size_t user_id_t;

// time_info stores the time information of a purchase:
struct time_info {
  // the time of a purchase (not necessary if purchases come in time order)
  uint64_t purchase_time;
  // the order of a purchase when it is processed
  std::size_t purchase_order;
};

// purchase_info stores the information of a purchase:
struct purchase_info {
  // time information of a purchase
  time_info tm_info;
  // purchase amount
  double amount;
};

// user_info class stores the user's direct friends (ids),
// and all the purchases (time, order, and amount) made by the user
class user_info {
  private:
    std::unordered_set<user_id_t> friends_{};
    std::deque<purchase_info> recent_purchases_{};

  public:
    // default constructor
    user_info() = default;

    // function to get the ids of the user's direct friends
    // return: an unordered_set containing ids of all friends
    const std::unordered_set<user_id_t>& get_friend_list() const {return friends_;}

    // function to get the most recent T purchases of the user
    // return: a deque containing the purchase information
    //         (purchase time, purchase order, and amount)
    const std::deque<purchase_info>& get_purchase_record() const {return recent_purchases_;}

    // function to add a friend to a user's friend list
    // input: a friend id
    void add_friend(const user_id_t friend_id) {friends_.insert(friend_id);}

    // function to remove a friend from a user's friend list
    // input: a friend id
    void remove_friend(const user_id_t friend_id) {friends_.erase(friend_id);}

    // function to add a recent purchase
    //         and remove the oldest one if the number of purchases is larger than T
    // inputs: timestamp - purchase time read from json file
    //         purchase_order - purchase order when the purchase is read from json file
    //         amount - purchase amount
    //         T - the number of the most recent purchases made in the user's network
    void update_purchases(const std::string& timestamp, const std::size_t purchase_order,
        const double amount, const std::size_t T);
};

#endif /* USER_INFO_H_ */
