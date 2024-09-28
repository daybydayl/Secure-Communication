#include "../include/Logger.h"

Logger	Logger::m_log;
Logger::Logger()
{
    m_writer.open("logger.log");
    m_level = INFO;
    m_device = CONSOLE;
}

Logger* Logger::get_instance()
{
    return &m_log;
}

int Logger::log_at_once(std::string text, std::string file, int line, Level level)
{
	bool flag = level >= m_level;
	if (flag)
	{
		if (m_device == CONSOLE)
		{
			// 日志信息控制台输出
			std::cout << "File[ " << file << " ], Line[ " << line
				<< " ], Infomation[ " << text << " ]" << std::endl;
		}
		else
		{
			// 日志信息写文件
			m_writer << "File[ " << file << " ], Line[ " << line
				<< " ], Infomation[ " << text << " ]" << std::endl;
			// 数据强制刷到磁盘
			m_writer.flush();
		}

	}
    return 0;
}

int Logger::set_enable_level(Level level)
{
    m_level = level;
    return 0;
}

Logger::~Logger()
{
    m_writer.close();
}
