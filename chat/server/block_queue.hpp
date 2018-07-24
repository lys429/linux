
#include<vector>
#include<semaphore.h>

namespace server{
  template<typename T>
  //模板类
  //阻塞队列，从后面添加消息++tail_
  //从前面删除消息--head_
    class BlockQueue
    {
      public:
        BlockQueue(size_t s = 1024)
          :data_(s)
		  ,head_(0)
		  ,tail_(0)
		  , size_(0)
        {
          sem_init(&sem_data_, 0, 0);//初始化空白和数据
          sem_init(&sem_blank_, 0, s);//将vector全部初始化为可用blank

        }
        ~BlockQueue()
        {
          sem_destroy(&sem_data_);
          sem_destroy(&sem_blank_);

        }
        void PushBack(const T&value)
        {
          sem_wait(&sem_blank_);//blank进行p操作（--）
          data_[tail_++] = value;
          if (tail_ >= data_.size())//如果满了从头开始
          {
            tail_ = 0;

          }
          ++size_;
          sem_post(&sem_data_);//V操作++

        }
        void PopFront(T*value)
        {
          sem_wait(&sem_data_);//数据进行p操作（--）
          *value = data_[head_++];
          if (head_ >= data_.size())
          {
            head_ = 0;

          }
          --size_;
          sem_post(&sem_blank_);//V操作++

        }
      private:
        std::vector<T> data_;
        sem_t sem_data_;
        sem_t sem_blank_;
		//sem_t sem_lock_;//二元信号互斥，在这里
		//是单消费者单生产者，可以不用加互斥锁
        size_t head_;
        size_t tail_;
        size_t size_;

    };


}//end server BlockQuelboost_system 
