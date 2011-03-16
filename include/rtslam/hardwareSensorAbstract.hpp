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
namespace hardware {

//namespace ublas = boost::numeric::ublas;

#if 0
class HardwareSensorAbstract;
typedef boost::shared_ptr<HardwareSensorAbstract> hardware_sensor_ptr_t;
#endif

#if 1

typedef struct { unsigned id; double timestamp; double arrival; } RawInfo;
typedef struct { std::vector<RawInfo> available; RawInfo next; double process_time; } RawInfos;

inline double getRawTimestamp(raw_ptr_t &raw) { return raw->timestamp; }
inline double getRawTimestamp(jblas::vec &raw) { return raw(0); }

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
		
	protected:
		kernel::VariableCondition<int> &condition; /// to notify when new data is available
		kernel::VariableCondition<int> index;
		boost::mutex mutex_data; /// mutex for using this object
		boost::condition_variable cond_offline_full;
		boost::condition_variable cond_offline_freed;
		int image_count; /// image count since last image read
		bool no_more_data;
		
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
			return read_pos;
		}
		int getLastUnreadPos() {
			return (write_pos == 0 ? bufferSize-1 : write_pos-1);
		}
		
	public:
		/** Constructor
			@param condition to notify when new data is available
		*/
		HardwareSensorAbstract(kernel::VariableCondition<int> &condition, unsigned bufferSize):
			write_pos(0), read_pos(bufferSize-1), condition(condition), index(0),
		  image_count(0), no_more_data(false), bufferSize(bufferSize), buffer(bufferSize)
		{}
		
		VecIndT getRaws(double t1, double t2); /// will also release the raws before the first one
		RawInfos getUnreadRawInfos(); /// get timing informations about unread raws
		void getRaw(unsigned id, T& raw); /// will also release the raws before this one
		int getLastUnreadRaw(T& raw); /// will also release the raws before this one
};


class HardwareSensorProprioAbstract: public HardwareSensorAbstract<jblas::vec>
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
	// TODO
	return RawInfos();
}

template<typename T>
void HardwareSensorAbstract<T>::getRaw(unsigned id, T& raw)
{
	read_pos = id;
	raw = buffer[read_pos];
}

template<typename T>
int HardwareSensorAbstract<T>::getLastUnreadRaw(T& raw)
{
	boost::unique_lock<boost::mutex> l(mutex_data);
	int missed_count = image_count-1;
	if (image_count > 0)
	{
		read_pos = getLastUnreadPos();
		raw = buffer[read_pos];
		image_count = 0;
		index.applyAndNotify(boost::lambda::_1++);
	}
	if (no_more_data && missed_count == -1) return -2; else return missed_count;
}



#endif


}}}

#endif

