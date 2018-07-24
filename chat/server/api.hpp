#pragma once
#include<strings.h>
#include<iostream>
#include<jsoncpp/json/json.h>
namespace server{

  struct Data
  {
    std::string name;
    std::string school;
    std::string msg;
    std::string cmd;
    void Serialize(std::string* output)//序列化
    {
      Json::Value value;
      value["name"] = name;
      value["school"] = school;
      value["msg"] = msg;
      value["cmd"] = cmd;
      Json::FastWriter writer;
      *output = writer.write(value);
      return;

    }
    void UnSerialize(const std::string& input)//反序列化
    {
      Json::Value value;
      Json::Reader reader;
      reader.parse(input, value);
      //if (value["name"].isString())//加入判定类型，更安全
	  name = value["name"].asString();
      school = value["school"].asString();
      msg = value["msg"].asString();
      cmd = value["cmd"].asString();
      return;

    }

  };

}
