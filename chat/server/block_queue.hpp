
#include<vector>
#include<semaphore.h>

namespace server{
  template<typename T>
  //ģ����
  //�������У��Ӻ��������Ϣ++tail_
  //��ǰ��ɾ����Ϣ--head_
    class BlockQueue
    {
      public:
        BlockQueue(size_t s = 1024)
          :data_(s)
		  ,head_(0)
		  ,tail_(0)
		  , size_(0)
        {
          sem_init(&sem_data_, 0, 0);//��ʼ���հ׺�����
          sem_init(&sem_blank_, 0, s);//��vectorȫ����ʼ��Ϊ����blank

        }
        ~BlockQueue()
        {
          sem_destroy(&sem_data_);
          sem_destroy(&sem_blank_);

        }
        void PushBack(const T&value)
        {
          sem_wait(&sem_blank_);//blank����p������--��
          data_[tail_++] = value;
          if (tail_ >= data_.size())//������˴�ͷ��ʼ
          {
            tail_ = 0;

          }
          ++size_;
          sem_post(&sem_data_);//V����++

        }
        void PopFront(T*value)
        {
          sem_wait(&sem_data_);//���ݽ���p������--��
          *value = data_[head_++];
          if (head_ >= data_.size())
          {
            head_ = 0;

          }
          --size_;
          sem_post(&sem_blank_);//V����++

        }
      private:
        std::vector<T> data_;
        sem_t sem_data_;
        sem_t sem_blank_;
		//sem_t sem_lock_;//��Ԫ�źŻ��⣬������
		//�ǵ������ߵ������ߣ����Բ��üӻ�����
        size_t head_;
        size_t tail_;
        size_t size_;

    };


}//end server BlockQuelboost_system 
