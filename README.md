# Cpp11_ThreadPool
基于C++11实现的简易优先级线程池  

项目描述：此项目是基于C++11实现的简易优先级线程池，支持动态调整线程池大小，支持按照任务的优先级进行调度。  

主要工作：  
1、运用模板技术，使得线程池能够接受不同类型的任务函数和参数，提高了代码的通用性；  

2、通过互斥锁和条件变量实现了线程间的同步和通信，确保任务的正确添加和执行；  

3、利用future和packaged_task异步获取任务的执行结果；  

4、线程池分为管理线程和工作线程，管理线程负责监控线程池的负载情况，动态调整工作线程数量。工作线程负责从任务队列中取出任务进行执行。 
