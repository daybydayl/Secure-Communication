#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <fstream>

//很简易版，待有需求再改

//饿汉模式单例类，线程安全
class Logger
{
public:
	enum Type {CONSOLE, FILE };
	enum Level {DEBUG, INFO, WARNING, ERROR, CRITICAL };
	static Logger* get_instance();
	int log_at_once(std::string text, std::string file, int line, Level = INFO);
	int set_enable_level(Level level);
	inline void set_device(Type dev_type)
	{
		m_device = dev_type;
	}
	~Logger();

private:
	Logger();

private:
	static Logger	m_log;
	std::ofstream	m_writer;
	Level			m_level;
	Type			m_device;


};

#endif // !LOGGER_H
