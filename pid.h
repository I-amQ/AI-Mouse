#include <chrono>

class PID
{
private:
	double prev_err = 0;
	double integral = 0;

public:
	PID(const double input_kp,const double input_ki,const double input_kd) : kp(input_kp), ki(input_ki), kd(input_kd) {};

	double kp = 0;
	double ki = 0;
	double kd = 0;
	
	void change_parameter(const double input_kp, const double input_ki, const double input_kd);
	void clear_stats();
	void clear_time();
	long step(double x);

	std::chrono::system_clock::time_point last_step = std::chrono::time_point<std::chrono::system_clock>{};
};