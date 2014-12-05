//
//  ConfigParser.h
//  SwiftSnails
//
//  Created by Chunwei on 12/2/14.
//  Copyright (c) 2014 Chunwei. All rights reserved.
//

#ifndef SwiftSnails_utils_ConfigParser_h_
#define SwiftSnails_utils_ConfigParser_h_
#include "common.h"
#include "string.h"

namespace swift_snails {
/*
 * 解析简单的配置文件
 *
 * 基本的语法：
 * import ../common.conf
 * key: value
 * # comment
 * key: value
 */
class ConfigParser : VirtualObject {

public:
    struct Item {
        std::string value;
        Item(const std::string &value) : value(value){ }
        int to_int32() const {
            CHECK(!value.empty());
            return std::move(stoi(value));
        }
        std::string to_string() const {
            return value;
        }
        bool to_bool() const {
            CHECK(!value.empty());
            CHECK(value == "true" || value == "false");
            if(value == "true") return true;
            return false;
        }
    };


    ConfigParser(const std::string &conf_path): _conf_path(conf_path) {
    }

    void register_config(const std::string &key, const std::string &value = "") {
        CHECK(!key.empty());
        CHECK(_dic.count(key) == 0) << "multi key:\t" << key << "is registered";
        _dic.insert({std::move(key), Item(value)});
    }

    // 返回值中支持操作：
    // to_int32()
    // to_string()
    // to_bool()
    const Item& get_config(const std::string &key) {
        auto p = _dic.find(key);
        CHECK(p != _dic.end()) << "no registered key:\t" << key;
        return p->second;
    }

    void parse() {
        LOG(WARNING) << "load conf from\t" << _conf_path;
        parse_conf(_conf_path);
    }

    friend std::ostream& operator<< (std::ostream& os, const ConfigParser &other) {
        os << "conf:" << std::endl;
        for(auto kv : other._dic) {
            os << kv.first << "\t" << kv.second.value << std::endl;
        }
        os << "end conf" << std::endl;
        return os;
    }

private:

    void parse_conf(const std::string &path) {
        LOG(INFO) << "parsing conf:\t" << path;
        typedef std::pair<std::string, std::string> key_value_t;
        std::ifstream file(path);
        PCHECK(file) << "conf can not open:\t" << path;
        std::string line;
        while(getline(file, line)) {
            trim(line);
            if(line.empty()) continue;
            if(headswith(line, "#")) continue;  // skip comments
            // import path
            if(headswith(line, "import")) {
                std::pair<std::string, std::string> kv = key_value_split(line, " ");
                CHECK(kv.second != path);   // 还需要防止交叉循环import
                parse_conf(kv.second);
                continue;
            }
            key_value_t kv = std::move(key_value_split(line, ":"));
            set_config(trim(kv.first), trim(kv.second));
        }
        file.close();
    }

    void set_config(const std::string &key, const std::string &value) {
        auto p = _dic.find(key);
        CHECK(p != _dic.end()) << "read unregistered key:\t" << key;
        p->second.value = value;
    }

    std::map<std::string, Item> _dic;
    std::string _conf_path;
};  // end class ConfigParser


}; // end namespace swift_snails

#endif