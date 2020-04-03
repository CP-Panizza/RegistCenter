//
// Created by cmj on 20-3-26.
//

#include <iostream>
#include <vector>
#include "Request.h"
#include "util.h"


void Request::Paser(std::string data) {
    auto x = split(data, "\r\n\r\n");
    if (x.size() != 2) {
        throw std::string("paser data err!");
    }
    if (x[1] != "") {
        this->body = x[1];
    }
    bool has_url_param = false;
    auto all = split(x[0], "\r\n");
    auto temp = all[0];
    auto line = split(temp, " ");
    if (line.size() != 3) {
        throw std::string("data err!");
    }
    this->method = line[0];
    this->url = line[1];
    this->http_version = line[2];
    auto v_path = split(line[1], "?");
    if (v_path.size() == 2) {
        this->path = v_path[0];
        has_url_param = true;
    } else if (v_path.size() == 1) {
        this->path = url;
    }

    if (has_url_param) {
        auto p = split(v_path[1], "&");
        for (auto y : p) {
            auto param = split(y, "=");
            if (param.size() == 2) {
                trim_space(param[0]);
                trim_space(param[1]);
                params[param[0]] = param[1];
            }
        }
    }

    for (int i = 1; i < all.size(); ++i) {
        if (all[i] != "" && contain(all[i], ":")) {
            auto head = split(all[i], ":");
            if (head.size() == 2) {
                trim_space(head[0]);
                trim_space(head[1]);
                this->header[head[0]] = head[1];
            }
        }
    }

}
