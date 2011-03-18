/**
 * \file hardwareSensorAbstract.hpp
 *
 * Header file for hardware sensors
 *
 * \date 18/06/2010
 * \author croussil
 *
 * \ingroup rtslam
 */

#ifndef HARDWARE_SENSOR_HPP_
#define HARDWARE_SENSOR_HPP_

#include "kernel/threads.hpp"

#include "jmath/indirectArray.hpp"

#include "rtslam/rawAbstract.hpp"

namespace jafar {
namespace rtslam {


struct RawInfo
{ unsigned id; double timestamp; double arrival;
	RawInfo(unsigned id, double timestamp, double arrival): id(id), timestamp(timestamp), arrival(arrival) {}
	RawInfo() {}
};
struct RawInfos
	{ std::vector<RawInfo> available; RawInfo next; double process_time; };

struct RawVec
{
	jblas::vec data;
	double arrival;
	RawVec(unsigned n): data(n), arrival(0.) {}
	RawVec() {}
};

namespace hardware {

//namespace ublas = boost::numeric::ublas;

#if 0
class HardwareSensorAbstract;
typedef boost::shared_ptr<HardwareSensorAbstract> hardware_sensor_ptr_t;
#endif

#if 1

inline double getRawTimestamp(raw_ptr_t &raw) { return raw->timestamp; }
inline double getRawTimestamp(RawVec &raw) { return raw.data(0); }
inline double getRawArrival(raw_ptr_t &raw) { return raw->arrival; }
inline double getRawArrival(RawVec &raw) { return raw.arrival; }

/**
	Generic implementation of hardware sensor based on ring buffer.
	You need to inherit this class and start in the constructor a thread that will
	read the sensor, fill the ring buffer, and notify the condition variable when
	a new reading arrives.
	
	TODO should be improved to be able to not fail if it overflows and that it should not
	be fatal but just throw away oldest data. 3 policies on overflow:
	fail, ignore/loop, block (for offline)
	
	@ingroup rtslam
*/
template<typename T>
class HardwareSensorAbstract
{
	public:
		typedef ublas::vector<T> VecT;
		typedef ublas::vector_indirect<VecT> VecIndT;
	
	private:
		int write_pos; /// next position where to write, oldest available reading
		int read_pos;  /// oldest position not released (being read or not read at all)
		
		int last_sent_pos; /// position of the last raw sent
		
	protected:
		kernel::VariableCondition<int> &condition; /// to notify when new data is available
		kernel::VariableCondition<int> index;
		boost::mutex mutex_data; /// mutex for using this object
		boost::condition_variable cond_offline_full;
		boost::condition_variable cond_offline_freed;
		int image_count; /// image count since last image read
		bool no_more_data;
		double timestamps_correction;
		double data_period;
		double arrival_delay;
		
		int bufferSize; /// size of the ring buffer
		VecT buffer; /// the ring buffer
		
		int getWritePos() {
			// don't need to lock, because will only be used and modified by writer
			return write_pos;
		}
		void incWritePos() {
			boost::unique_lock<boost::mutex> l(mutex_data);
			++write_pos;
			if (write_pos >= bufferSize) write_pos = 0;
			++image_count;
		}
		int getFirstUnreadPos() {
			// don't need to lock, because will only be used and modified by reader
			return read_pos;
		}
		int getLastUnreadPos() {
			boost::unique_lock<boost::mutex> l(mutex_data);
			return (write_pos == 0 ? bufferSize-1 : write_pos-1);
		}
		/// release until id, excluding id
		void releaseUntil(unsigned id) {
			boost::unique_lock<boost::mutex> l(mutex_data);
			read_pos = id;
		}
		/// release until id, including id
		void release(unsigned id) {
			boost::unique_lock<boost::mutex> l(mutex_data);
			read_pos = id+1;
			if (read_pos >= bufferSize) read_pos = 0;
		}
		bool isFull()
		{
			boost::unique_lock<boost::mutex> l(mutex_data);
			return (read_pos == write_pos);
		}
		
		/**
			Provides approximate informations about the timings of data
			@param data_period the period at which the data are arriving
			@param arrival_delay the delay between the moment we get a data and its real date.
			This is a starting point that must be overestimated,
			it may be estimated more precisely afterwards.
		*/
		virtual void getTimingInfos(double &data_period, double &arrival_delay)
			{ data_period = this->data_period; arrival_delay = this->arrival_delay; }
		virtual void setTimingInfos(double data_period, double arrival_delay)
			{ this->data_period = data_period; this->arrival_delay = arrival_delay; }
		
	public:
		/** Constructor
			@param condition to notify when new data is available
		*/
		HardwareSensorAbstract(kernel::VariableCondition<int> &condition, unsigned bufferSize):
			write_pos(0), read_pos(bufferSize-1), last_sent_pos(-1), condition(condition), index(0),
		  image_count(0), no_more_data(false), timestamps_correction(0.0),
		  bufferSize(bufferSize), buffer(bufferSize)
		{}
		void setSyncConfig(double timestamps_correction = 0.0)
			{ this->timestamps_correction = timestamps_correction; }
		
		VecIndT getRaws(double t1, double t2); /// will also release the raws before the first one
		RawInfos getUnreadRawInfos(); /// get timing informations about unread raws
		void getRaw(unsigned id, T& raw); /// will also release the raws before this one
		int getLastUnreadRaw(T& raw); /// will also release the raws before this one
		void getLastProcessedRaw(T& raw) { raw = buffer[last_sent_pos]; } /// for information only (display...)
};


class HardwareSensorProprioAbstract: public HardwareSensorAbstract<RawVec>
{
	public:
	/*
		TODO return some automatic information about what is provided
		
	enum locpart {
		x=1,y=2,z=4,pos=7, 
		qw=8,qx=16,qy=32,qz=64,q=120,
		vx=128,vy=256,vz=512,vel=903,
		vyaw=1024,vpitch=2048,vroll=4096,vela=7288
	};
	locpart bor(locpart a, locpart b) { return (locpart)((unsigned)a | (unsigned)b); }
	locpart bor(locpart a, locpart b, locpart c) { return (locpart)((unsigned)bor(a,b) | (unsigned)c); }
	locpart bor(locpart a, locpart b, locpart c, locpart d) { return (locpart)((unsigned)bor(a,b,c) | (unsigned)d); }
	locpart bor(locpart a, locpart b, locpart c, locpart d, locpart e) { return (locpart)((unsigned)bor(a,b,c,d) | (unsigned)e); }
	locpart bor(locpart a, locpart b, locpart c, locpart d, locpart e, locpart f) { return (locpart)((unsigned)bor(a,b,c,d,e) | (unsigned)f); }
	
	virtual ind_array get(locpart x);
	*/
	
		HardwareSensorProprioAbstract(kernel::VariableCondition<int> &condition, unsigned bufferSize):
			HardwareSensorAbstract(condition, bufferSize) {}
		virtual int dataSize() = 0; /// number of measure variables provided (without timestamp and variance)
		virtual int varianceSize() = 0; /// number of variance variables provided
	
	
};

class HardwareSensorExteroAbstract: public HardwareSensorAbstract<raw_ptr_t>
{
	public:
		HardwareSensorExteroAbstract(kernel::VariableCondition<int> &condition, unsigned bufferSize):
			HardwareSensorAbstract(condition, bufferSize) {}
	
	
};

typedef boost::shared_ptr<hardware::HardwareSensorExteroAbstract> hardware_sensorext_ptr_t;
typedef boost::shared_ptr<hardware::HardwareSensorProprioAbstract> hardware_sensorprop_ptr_t;

#endif

#if 0
class HardwareSensorAbstract
{
	protected:
		boost::condition_variable &rawdata_condition;
		boost::mutex &rawdata_mutex;
	public:
		HardwareSensorAbstract(boost::condition_variable &rawdata_condition, boost::mutex &rawdata_mutex):
			rawdata_condition(rawdata_condition), rawdata_mutex(rawdata_mutex) {}
		/**
		@param rawPtr the latest raw available from the sensor
		@return the number of missed raws, -1 if no raw is available since last call, -2 if no raw will ever be available
		@note must be non blocking
		*/
		virtual int acquireRaw(raw_ptr_t &rawPtr) = 0;
		virtual ~HardwareSensorAbstract() {}
};
#endif


#if 1

template<typename T>
typename HardwareSensorAbstract<T>::VecIndT HardwareSensorAbstract<T>::getRaws(double t1, double t2)
{
	JFR_ASSERT(t1 <= t2, "");
	boost::unique_lock<boost::mutex> l(mutex_data);
	int i1, i2;
	int i, j;

	// find indexes by dichotomy
	int i_left = write_pos, i_right = write_pos + bufferSize-1;
	while(i_left != i_right)
	{
		j = (i_left+i_right)/2;
		i = j % bufferSize;
		if (getRawTimestamp(buffer(i)) >= t1) i_right = j; else i_left = j+1;
	}
	i = i_left % bufferSize;
	i1 = (i-1 + bufferSize) % bufferSize;
	if (t1 <= 1.0 && getRawTimestamp(buffer(i1)) < 0.0) i1 = i;
	bool no_larger = (getRawTimestamp(buffer(i)) < t1);
	bool no_smaller = (i == write_pos);
	if (no_larger && getRawTimestamp(buffer(i1)) < 0.0)  // no data at all
		return ublas::project(buffer, jmath::ublasExtra::ia_set(ublas::range(0,0)));
	if (no_smaller) JFR_ERROR(RtslamException, RtslamException::BUFFER_OVERFLOW, "Missing data: increase buffer size !");
	
	if (no_larger)
		i2 = i1;
	else
	{
		i_right = write_pos + bufferSize-1;
		while(i_left != i_right)
		{
			j = (i_left+i_right)/2;
			i = j % bufferSize;
			if (getRawTimestamp(buffer(i)) >= t2) i_right = j; else i_left = j+1;
		}
		i = i_left % bufferSize;
		i2 = i;
	}
	
	
	// return mat_indirect
	read_pos = i1;
	l.unlock();
	cond_offline_freed.notify_all();

	if (i1 < i2)
	{
		return ublas::project(buffer, 
			jmath::ublasExtra::ia_set(ublas::range(i1,i2+1)));
	} else
	{
		return ublas::project(buffer, 
			jmath::ublasExtra::ia_concat(jmath::ublasExtra::ia_set(ublas::range(i1,buffer.size1())),
			                             jmath::ublasExtra::ia_set(ublas::range(0,i2+1))));
	}
}


template<typename T>
RawInfos HardwareSensorAbstract<T>::getUnreadRawInfos()
{
	RawInfos result;
	int first_stop, second_stop;
	int first = getFirstUnreadPos(), last = getLastUnreadPos();
	if (first <= last)
	{
		first_stop = last;
		second_stop = -1;
	} else
	{
		first_stop = bufferSize-1;
		second_stop = last;
	}
	
	for(int pos = first; pos <= first_stop; ++pos)
		result.available.push_back(RawInfo(pos,getRawTimestamp(buffer(pos)),getRawArrival(buffer(pos))));
	for(int pos = 0; pos <= second_stop; ++pos)
		result.available.push_back(RawInfo(pos,getRawTimestamp(buffer(pos)),getRawArrival(buffer(pos))));

	double data_period, arrival_delay;
	getTimingInfos(data_period, arrival_delay);
	data_period += result.available[result.available.size()-1].timestamp;
	result.next = RawInfo(0,data_period,data_period+arrival_delay);
	result.process_time = 0.;
	
	return result;
}

template<typename T>
void HardwareSensorAbstract<T>::getRaw(unsigned id, T& raw)
{
	releaseUntil(id);
	raw = buffer[id];
	last_sent_pos = id;
}

template<typename T>
int HardwareSensorAbstract<T>::getLastUnreadRaw(T& raw)
{
	boost::unique_lock<boost::mutex> l(mutex_data);
	int missed_count = image_count-1;
	if (image_count > 0)
	{
		unsigned id = getLastUnreadPos();
		releaseUntil(id);
		last_sent_pos = id;
		raw = buffer[id];
		image_count = 0;
		index.applyAndNotify(boost::lambda::_1++);
	}
	if (no_more_data && missed_count == -1) return -2; else return missed_count;
}


#endif


}}}

#endif

