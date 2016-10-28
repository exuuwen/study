#include "Rwlock.h"

Rwlock::Rwlock()
	: mutex_()
	, cond_(mutex_)
	, readers_(0)
	, writer_(false)
{
	
}

Rwlock::~Rwlock()
{
}

void Rwlock::rlock()
{
	mutex_.lock();
	while (writer_)	
	{
		cond_.wait();
	}

	readers_++;
	mutex_.unlock();
}

void Rwlock::runlock()
{
	mutex_.lock();

	readers_--;
	if (readers_ == 0)
	{
		cond_.broadcast();
	}

	mutex_.unlock();
}

void Rwlock::wlock()
{
	mutex_.lock();

	while (writer_)
	{
		cond_.wait();
	}	

	writer_ = true;

	while (readers_ != 0)
	{
		cond_.wait();	
	}

	mutex_.unlock();
}

void Rwlock::wunlock()
{
	mutex_.lock();

	writer_ = false;
	
	cond_.broadcast();

	mutex_.unlock();
}

