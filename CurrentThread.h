#pragma once

namespace CurrentThread
{
    /*
    * extern表示该变量是在其他文件中定义的，这里只是引用
    * __thread表示变量是线程局部存储的，每个线程拥有该变量的独立副本，一个线程修改该变量不影响其他线程
    */
    extern __thread int t_cachedTid;  // 用于缓存线程id
    void cacheTid();
    inline int tid()
    {
        if(__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}