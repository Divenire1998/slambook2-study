///对map.h中声明的各类函数进行定义实现

#include "myslam/map.h"
#include "myslam/feature.h"

namespace myslam
{

    // RemoveOldKeyframe操作过后，有些帧被舍弃了，里面的特征观测也全部被丢弃，这样就可能造成有些landmark没有观测了
    // 
    void Map::CleanMap()
    { 
        
        int cnt_landmark_removed = 0;

        for (auto iter = active_landmarks_.begin();
             iter != active_landmarks_.end();)
        {
            // 如果这个地图点没有被观测到,也就是说没有对应的特征点
            // 则将这个landmark从active_landmarks_中移除
            if (iter->second->observed_times_ == 0)
            {   
                iter = active_landmarks_.erase(iter);
                cnt_landmark_removed++;
            }
            else
            {
                ++iter;
            }
        }

        LOG(INFO) << "Removed " << cnt_landmark_removed << " active landmarks";
    }

    void Map::RemoveOldKeyframe()
    {
        if (current_frame_ == nullptr)
            return;

        // 当前帧的位姿取出来 SE3
        auto Twc = current_frame_->Pose().inverse();

        // 寻找与当前帧最近与最远的两个关键帧
        // se3的代数空间上的距离.
        // TODO 这样好吗?为什么不直接取平移呢?
        double max_dis = 0, min_dis = 9999;
        double max_kf_id = 0, min_kf_id = 0;

        // 计算激活窗口内的每一帧与当前帧的se3差异
        // 求得最大的距离与最小的距离
        for (auto &kf : active_keyframes_)
        {

            if (kf.second == current_frame_)
                continue;

            // 两帧之间se3的差异取se3差异的模作为距离
            auto dis = (kf.second->Pose() * Twc).log().norm();

            // 计算最大的距离
            if (dis > max_dis)
            {
                max_dis = dis;
                max_kf_id = kf.first;
            }

            // 计算最小的距离
            if (dis < min_dis)
            {
                min_dis = dis;
                min_kf_id = kf.first;
            }
        }

        // TODO 这个阈值怎么确定的?  测量一下静止的时候的值?
        // 两帧之间的se3变化值
        const double min_dis_th = 0.2; 

        Frame::Ptr frame_to_remove = nullptr;

        // * 剔除策略
        // 如果距离小于最近的阈值,则认为这个帧携带的信息很少,剔除最近的关键帧
        // 否则剔除最远的关键帧
        // 如果前端跟踪的内点小于 ** 就认为是关键帧
        if (min_dis < min_dis_th)
        {
            // 如果存在很近的帧，优先删掉最近的
            frame_to_remove = keyframes_.at(min_kf_id);
        }
        else
        {
            // 删掉最远的
            frame_to_remove = keyframes_.at(max_kf_id);
        }

        LOG(INFO) << "remove keyframe " << frame_to_remove->keyframe_id_;

        // 关键帧被删除了,关键帧包含了 特征 和 地图点
        // 之前与地图点建立了联系的特征

        // step 1 ,根据id号,将滑动窗口内旧的关键帧给移除
        active_keyframes_.erase(frame_to_remove->keyframe_id_);

        // Step 2 移除旧的关键帧的观测
        for (auto feat : frame_to_remove->features_left_)
        {
            // 返回特征点对应地图点的指针
            auto mp = feat->map_point_.lock();

            // 如果这个地图点还存在,移除特征对该地图点的观测
            // 比如原来mapPoint 被 feature 1和 feature2看到,现在看到不了.

            // 清除地图点和特征的双向链接关系
            // 同时刷新地图点的被观测次数
            if (mp)
            {
                mp->RemoveObservation(feat);
            }
        }

        // 右目同理
        for (auto feat : frame_to_remove->features_right_)
        {
            if (feat == nullptr)
                continue;
            auto mp = feat->map_point_.lock();
            if (mp)
            {
                mp->RemoveObservation(feat);
            }
        }

        // 清空地图中没有被任何帧观测到的地图点。
        // 也就是这个地图点没有任何一个关联的特征
        CleanMap();
    }

    // 插入关键帧，并移除旧的关键帧(最远或者最近)
    void Map::InsertKeyFrame(Frame::Ptr frame)
    {

        // typedef std::unordered_map<unsigned long, Frame::Ptr> KeyframesType;
        //根据Map类中定义的这个unordered_map容器，我们可知插入一个关键帧需要分配给其一个容器内的索引值

        // ! Frontend和mappoint两个类里都有一个叫做current_frame_的私有成员变量，这里要注意和Frontend里面的区分
        current_frame_ = frame;

        //如果key存在，则find返回key对应的迭代器，如果key不存在，则find返回unordered_map::end。
        if (keyframes_.find(frame->keyframe_id_) == keyframes_.end())
        {

            // 关键帧插入
            keyframes_.insert(make_pair(frame->keyframe_id_, frame));

            // 新插入的关键帧是时间上最新的，所以肯定是当前的激活关键帧
            // 激活关键帧会参与显示和后端优化
            active_keyframes_.insert(make_pair(frame->keyframe_id_, frame));
        }
        else
        {
            // ?这儿其实根本不会发生吧
            //找到了这个关键帧编号，则以当前要插入的关键帧对原有内容覆盖
            keyframes_[frame->keyframe_id_] = frame;
            active_keyframes_[frame->keyframe_id_] = frame;
        }

        // num_active_keyframes_ = 7
        // 如果当前激活的关键帧大于7个,就要移除最老的关键帧
        if (active_keyframes_.size() > num_active_keyframes_)
        {
            RemoveOldKeyframe();
        }
    }

    void Map::InsertMapPoint(MapPoint::Ptr map_point)
    { 
        //与插入关键帧同理
        if (landmarks_.find(map_point->id_) == landmarks_.end())
        {
            landmarks_.insert(make_pair(map_point->id_, map_point));

            // 刚刚插入进来的地图点，肯定是能看到的
            active_landmarks_.insert(make_pair(map_point->id_, map_point));
        }
        else
        {
            // 如果这个地图点已经存在了
            // ? 这个新手程序其实是跑不到这里的
            // 因为所有的地图点都是左右目匹配新生成的
            // 就算是相同的特征，跟丢以后再找回来还是会新建一个地图点
            landmarks_[map_point->id_] = map_point;
            active_landmarks_[map_point->id_] = map_point;
        }
    }
}
