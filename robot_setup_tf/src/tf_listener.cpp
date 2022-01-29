#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/PointStamped.h>

struct point3d{
    float x;
    float y;
    float z;
};

/* transform为base_link指向base_laser
** 所以base_link -> base_laser + base_laser -> local at base_laser = local at base_link
** 三维点不涉及姿态
*/
point3d operator*(const geometry_msgs::PointStamped point, const tf::StampedTransform transform){
    point3d pt;
    pt.x = point.point.x + transform.getOrigin().x();
    pt.y = point.point.y + transform.getOrigin().y();
    pt.z = point.point.z + transform.getOrigin().z();
    return pt;
}

void transformPoint(const tf::TransformListener& listener){
    geometry_msgs::PointStamped laser_point;
    laser_point.header.frame_id = "base_laser";
    laser_point.header.stamp = ros::Time();

    laser_point.point.x = 1.0;
    laser_point.point.y = 0.2;
    laser_point.point.z = 0.0;

    try{
        // 以下是官网教程用法
        /******
        geometry_msgs::PointStamped base_point;
        listener.transformPoint("base_link", laser_point, base_point);

        ROS_INFO("base_laser: (%.2f, %.2f. %.2f) -----> base_link: (%.2f, %.2f, %.2f) at time %.2f",
            laser_point.point.x, laser_point.point.y, laser_point.point.z,
            base_point.point.x, base_point.point.y, base_point.point.z, base_point.header.stamp.toSec()
        );
        ******/

        // note: 按照先前的学习, 通过lookupTransform获得两个坐标系的坐标变换, getOrigin()返回平移, btVector3类型, getRotation()返回旋转, btQuaternion类型
        // getBasis()可以获得旋转矩阵
        tf::StampedTransform transform;
        listener.lookupTransform("base_link", "base_laser", ros::Time(0), transform);
        point3d base_point = laser_point * transform;
        ROS_INFO("base_laser: (%.2f, %.2f. %.2f) -----> base_link: (%.2f, %.2f, %.2f)",
            laser_point.point.x, laser_point.point.y, laser_point.point.z,
            base_point.x, base_point.y, base_point.z
        );
    }

    catch(tf::TransformException& ex){
        ROS_ERROR("Received an exception trying to transform a point from \"base_laser\" to \"base_link\": %s", ex.what());
    }
}

int main(int argc, char** argv){
    ros::init(argc, argv, "robot_tf_listener");
    ros::NodeHandle n;

    tf::TransformListener listener(ros::Duration(10));

    ros::Timer timer = n.createTimer(ros::Duration(1.0), boost::bind(&transformPoint, boost::ref(listener)));
    
    ros::spin();
    return 0;
}
