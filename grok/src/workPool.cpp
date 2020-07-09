#include "../include/workPool.h"

using namespace grok;

void WorkPool::insertJob(const WorkPool::Keys& keys, std::function<void()> f) {
	{
		std::lock_guard<std::mutex> guard(m_lock);
		for (auto& it : keys) {
			if (m_keyRunnings.count(it)) {
				// 发现前面居然已经有在等待的处理了，就追加到队列后面等待吧
				Job::Ptr job = std::make_shared<Job>();
				job->keys = keys;
				job->jobcb = f;
				m_jobs.push_back(job);
				return;
			}
		}

		// 到了这里，说明keys 中的所有key都没有被占用，那它立刻就可以执行了
		// 一旦立刻执行了，就可以把key设置到 m_keyRunnings 中了
		for (auto& it : keys)
		{
			m_keyRunnings.insert(it);
		}
	}

	// 走到了这里，就代表可以直接调用了！！
	this->ios().post(f);
}

void WorkPool::completeJob(const WorkPool::Keys& keys) {
	std::vector<Job::Ptr> pendingjobs;
	{
		std::lock_guard<std::mutex> guard(m_lock);
		// 把当前的key还原掉
		for (auto& it : keys)
		{
			m_keyRunnings.erase(it);
		}

		// 当m_keyRunnings的状态发生变化了，就要检查一下，m_jobs中的任务是否有可以执行的了
		std::set<std::string> preKeys;
		for (auto it = m_jobs.begin(); it != m_jobs.end();)
		{
			bool ok = true;
			for (auto& it1 : (*it)->keys)
			{
				if (m_keyRunnings.count(it1) || preKeys.count(it1)) {
					ok = false;
					break;
				}
				// 保存前面Job的key，后面的Job如果也有这个key,要等前面的job先执行
				preKeys.insert(it1);
			}
			if (ok) {
				for (auto& it1 : (*it)->keys)
				{
					m_keyRunnings.insert(it1);
				}
				// 推送到pending队列，需要在锁外面触发调用
				pendingjobs.push_back(*it);
				// 唤醒执行过之后，就代表这个任务单可以完结了
				it = m_jobs.erase(it);
			}
			else {
				++it;
			}
		}
	}
	for (auto it = pendingjobs.begin(); it != pendingjobs.end(); ++it)
	{
		// 唤醒等待的Job
		this->ios().post((*it)->jobcb);
	}
}