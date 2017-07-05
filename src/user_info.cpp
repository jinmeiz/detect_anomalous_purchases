/*
 * user_info.cpp
 *
 *  Created on: Jul 1, 2017
 *      Author: jinmei
 */

#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "user_info.h"

// function to convert a time in string type to uint64_t type
uint64_t convert_string2timet(std::string timestamp) {
  auto end = std::remove_if(timestamp.begin(), timestamp.end(), [] (const char c) {
    return ! std::isdigit(c);
  });
  timestamp.resize(end - timestamp.begin());
  uint64_t time = std::stol(timestamp);
  return time;
}

void user_info::update_purchases(const std::string& timestamp,
    const std::size_t purchase_order,
    const double amount, const std::size_t T) {

  // convert purchase time into C uint64_t type
  uint64_t purchase_time = convert_string2timet(timestamp);

  time_info purchase_time_info {purchase_time, purchase_order};
  purchase_info purchase_info {purchase_time_info, amount};
  recent_purchases_.push_front(purchase_info);

//  // If purchases do not come in time order,
//  // replace recent_purchases_.push_front(purchase_info) with the following codes
//  if (recent_purchases_.empty() ||
//      purchase_time >= recent_purchases_.front().tm_info.purchase_time) {
//    // when the purchase is most recent, add it to the front of the record
//    recent_purchases_.push_front(purchase_info);
//
//  } else if (purchase_time < recent_purchases_.back().tm_info.purchase_time) {
//    // when the purchase is the oldest, add it to the back of the record
//    recent_purchases_.push_front(purchase_info);
//
//  } else {
//    // add the purchase in front of the one that has an equal or earlier time stamp
//    for (auto iter = recent_purchases_.begin(); iter != recent_purchases_.end();
//        ++iter) {
//      if (purchase_time >= iter->time_info.purchase_time) {
//        recent_purchases_.insert(iter, purchase_info);
//        break;
//      }
//    } // end of for loop
//  }

  // if the number of purchases is larger than T, remove the oldest purchase
  if (recent_purchases_.size() > T)
    recent_purchases_.pop_back();
}

