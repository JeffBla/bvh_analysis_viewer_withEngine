#include "Skeleton.h"

#include <cmath>

using namespace skeleton;
using namespace bone;


Bone *Skeleton::CreateBone(const string &bone_name, Bone *parent, rp3d::Vector3 &pos,
                           const rp3d::Quaternion &orientation, const openglframework::Vector3 &size,
                           rp3d::decimal massDensity, const string &model_file) {
    PhysicsObject *boneObject = CreateBonePhysics(pos, orientation, size, massDensity, model_file);

    auto new_bone = new Bone(bone_name, boneObject, BoneType::CONE, pos, parent);
    bones[bone_name] = new_bone;

    if (parent != nullptr) {
        parent->AppendChild(new_bone);
    }
    return new_bone;
}

Bone *Skeleton::CreateBone(const string &bone_name, Bone *parent, rp3d::Vector3 &pos,
                           const rp3d::Quaternion &orientation, float radius, rp3d::decimal massDensity) {
    PhysicsObject *boneObject = CreateBonePhysics(pos, orientation, radius, massDensity);

    auto new_bone = new Bone(bone_name, boneObject, BoneType::SPHERE, pos, parent);
    bones[bone_name] = new_bone;

    if (parent != nullptr) {
        parent->AppendChild(new_bone);
    }
    return new_bone;
}

void
Skeleton::ConfigNewObject(PhysicsObject *new_object, const rp3d::Vector3 &pos, const rp3d::Quaternion &orientation) {
    new_object->setTransform(rp3d::Transform(pos, orientation));

    // Set the box color
    new_object->setColor(objectColor);
    new_object->setSleepingColor(sleepingColor);

    new_object->getRigidBody()->updateMassPropertiesFromColliders();
    new_object->getRigidBody()->setLinearDamping(linearDamping);
    new_object->getRigidBody()->setAngularDamping(angularDamping);
    new_object->getRigidBody()->setType(rp3d::BodyType::KINEMATIC);
}

ConvexMesh *
Skeleton::CreateBonePhysics(const rp3d::Vector3 &pos, const rp3d::Quaternion &orientation,
                            const openglframework::Vector3 &size, rp3d::decimal massDensity,
                            const string &model_file) {
    auto *new_bone_object = new ConvexMesh(true, mPhysicsCommon, mPhysicsWorld, mMeshFolderPath + model_file,
                                           {size.x, size.y, size.z});
    new_bone_object->getCollider()->getMaterial().setMassDensity(massDensity);
    new_bone_object->getCollider()->getMaterial().setFrictionCoefficient(frictionCoeff);
    ConfigNewObject(new_bone_object, pos, orientation);
    mPhysicsObjects.push_back(new_bone_object);

    return new_bone_object;
}

Sphere *Skeleton::CreateBonePhysics(const rp3d::Vector3 &pos, const rp3d::Quaternion &orientation, float radius,
                                    rp3d::decimal massDensity) {
    auto *new_bone_object = new Sphere(true, radius, mPhysicsCommon, mPhysicsWorld, mMeshFolderPath);
    new_bone_object->getCollider()->getMaterial().setMassDensity(massDensity);
    new_bone_object->getCollider()->getMaterial().setFrictionCoefficient(frictionCoeff);
    ConfigNewObject(new_bone_object, pos, orientation);
    mPhysicsObjects.push_back(new_bone_object);

    return new_bone_object;
}

Skeleton::Skeleton(rp3d::PhysicsCommon &mPhysicsCommon, rp3d::PhysicsWorld *mPhysicsWorld,
                   vector<PhysicsObject *> &mPhysicsObjects, std::string &mMeshFolderPath,
                   const bvh::Joint *hipJoint)
        : mPhysicsCommon(mPhysicsCommon), mPhysicsWorld(mPhysicsWorld), mPhysicsObjects(mPhysicsObjects),
          mMeshFolderPath(mMeshFolderPath) {

    rp3d::Vector3 ragdollPosition(0, 0, 0);
    rp3d::Vector3 defaultPosition(0, 0, 0);
    float mHip_radius = 0.2f;

    // --------------- Create the hips Sphere --------------- //
    mHipBone = CreateBone("hip", nullptr, ragdollPosition, rp3d::Quaternion::identity(),
                          mHip_radius, 20);
    mHipBone->GetPhysicsObject()->getRigidBody()->setType(rp3d::BodyType::STATIC);

    // --------------- Create the hips left Cone --------------- //
    mHipLeftBone = CreateBone("hipLeft", mHipBone, defaultPosition,
                              rp3d::Quaternion::fromEulerAngles(0, 0, -rp3d::PI_RP3D / 3.0),
                              {0.15, 1, 0.15}, 9,
                              "cone_offset.obj");

    // --------------- Create the hips right Cone --------------- //
    mHipRightBone = CreateBone("hipRight", mHipBone, defaultPosition,
                               rp3d::Quaternion::fromEulerAngles(0, 0, rp3d::PI_RP3D / 3.0),
                               {0.15, 1, 0.15}, 9,
                               "cone_offset.obj");

    // --------------- Create the waist Cone --------------- //
    mWaistBone = CreateBone("waist", mHipBone, defaultPosition, rp3d::Quaternion::identity(),
                            {0.2, 2, 0.2}, 9,
                            "cone_offset.obj");

    // --------------- Create the chest Cone --------------- //
    mChestBone = CreateBone("chest", mWaistBone, defaultPosition, rp3d::Quaternion::identity(),
                            {0.2, 1.5, 0.2}, 9,
                            "cone_offset.obj");

    // --------------- Create the chest left Cone --------------- //
    mChestLeftBone = CreateBone("chestLeft", mWaistBone, defaultPosition,
                                rp3d::Quaternion::fromEulerAngles(0, 0, -rp3d::PI_RP3D / 10.0),
                                {0.2, 1.5, 0.2}, 9,
                                "cone_offset.obj");

    // --------------- Create the chest right Cone --------------- //
    mChestRightBone = CreateBone("chestRight", mWaistBone, defaultPosition,
                                 rp3d::Quaternion::fromEulerAngles(0, 0, rp3d::PI_RP3D / 10.0),
                                 {0.2, 1.5, 0.2}, 9,
                                 "cone_offset.obj");

    // --------------- Create the neck Cone --------------- //
    mNeckBone = CreateBone("neck", mChestBone, defaultPosition, rp3d::Quaternion::identity(),
                           {0.2, 0.8, 0.2}, 9,
                           "cone_offset.obj");

    // --------------- Create the head Sphere --------------- //
    mHeadBone = CreateBone("head", mNeckBone, defaultPosition,
                           rp3d::Quaternion::identity(), 0.75f, 7);

    // --------------- Create the left shoulder Cone --------------- //
    mLeftShoulderBone = CreateBone("leftShoulder", mChestLeftBone, defaultPosition,
                                   rp3d::Quaternion::fromEulerAngles(0, 0, -rp3d::PI_RP3D / 1.8),
                                   {0.15, 1, 0.15}, 8, "cone_offset.obj");

    // --------------- Create the left upper arm Cone --------------- //
    mLeftUpperArmBone = CreateBone("leftUpperArm", mLeftShoulderBone, defaultPosition,
                                   rp3d::Quaternion::fromEulerAngles(0, 0, -rp3d::PI_RP3D / 2.0),
                                   {0.2, 2, 0.2}, 8, "cone_offset.obj");

    // --------------- Create the left lower arm Cone --------------- //
    mLeftLowerArmBone = CreateBone("leftLowerArm", mLeftUpperArmBone, defaultPosition,
                                   rp3d::Quaternion::fromEulerAngles(0, 0, -rp3d::PI_RP3D / 2.0),
                                   {0.2, 2, 0.2}, 8, "cone_offset.obj");

    // --------------- Create the left upper leg Cone --------------- //
    mLeftUpperLegBone = CreateBone("leftUpperLeg", mHipLeftBone, defaultPosition,
                                   rp3d::Quaternion::fromEulerAngles(rp3d::PI_RP3D, 0, 0),
                                   {0.2, 3, 0.2}, 8, "cone_offset.obj");

    // --------------- Create the left lower leg Cone --------------- //
    mLeftLowerLegBone = CreateBone("leftLowerLeg", mLeftUpperLegBone, defaultPosition,
                                   rp3d::Quaternion::fromEulerAngles(rp3d::PI_RP3D, 0, 0),
                                   {0.2, 3.5, 0.2}, 8, "cone_offset.obj");

    // --------------- Create the right shoulder Cone --------------- //
    mRightShoulderBone = CreateBone("rightShoulder", mChestRightBone, defaultPosition,
                                    rp3d::Quaternion::fromEulerAngles(0, 0, rp3d::PI_RP3D / 1.8),
                                    {0.15, 1, 0.15}, 8, "cone_offset.obj");

    // --------------- Create the right upper arm Cone --------------- //
    mRightUpperArmBone = CreateBone("rightUpperArm", mRightShoulderBone, defaultPosition,
                                    rp3d::Quaternion::fromEulerAngles(0, 0, rp3d::PI_RP3D / 2.0),
                                    {0.2, 2, 0.2}, 8, "cone_offset.obj");

    // --------------- Create the right lower arm Cone --------------- //
    mRightLowerArmBone = CreateBone("rightLowerArm", mRightUpperArmBone, defaultPosition,
                                    rp3d::Quaternion::fromEulerAngles(0, 0, rp3d::PI_RP3D / 2.0),
                                    {0.2, 2, 0.2}, 8, "cone_offset.obj");

    // --------------- Create the right upper leg Cone --------------- //
    mRightUpperLegBone = CreateBone("rightUpperLeg", mHipRightBone, defaultPosition,
                                    rp3d::Quaternion::fromEulerAngles(rp3d::PI_RP3D, 0, 0),
                                    {0.2, 3, 0.2}, 8, "cone_offset.obj");

    // --------------- Create the right lower leg Cone --------------- //
    mRightLowerLegBone = CreateBone("rightLowerLeg", mRightUpperLegBone, defaultPosition,
                                    rp3d::Quaternion::fromEulerAngles(rp3d::PI_RP3D, 0, 0),
                                    {0.2, 3.5, 0.2}, 8, "cone_offset.obj");

    mHipBone->UpdateChild();
    // --------------- Create the joint between head and chest --------------- //

    // Create the joint info object
    rp3d::RigidBody *body1 = mHeadBone->GetPhysicsObject()->getRigidBody();
    rp3d::RigidBody *body2 = mChestBone->GetPhysicsObject()->getRigidBody();
    rp3d::BallAndSocketJointInfo jointInfo1(body1, body2,
                                            mHipBone->GetPosition() + rp3d::Vector3(0, -0.75, 0));
    jointInfo1.isCollisionEnabled = false;
    mHeadChestJoint = dynamic_cast<rp3d::BallAndSocketJoint *>(mPhysicsWorld->createJoint(jointInfo1));
    mHeadChestJoint->setConeLimitHalfAngle(40.0 * rp3d::PI_RP3D / 180.0);
    mHeadChestJoint->enableConeLimit(true);

    // --------------- Create the joint between chest and left upper arm --------------- //

    // Create the joint info object
    body1 = mChestBone->GetPhysicsObject()->getRigidBody();
    body2 = mLeftUpperArmBone->GetPhysicsObject()->getRigidBody();
    rp3d::BallAndSocketJointInfo jointInfo2(body1, body2,
                                            mLeftUpperArmBone->GetPosition() + rp3d::Vector3(-1, 0, 0));
    jointInfo2.isCollisionEnabled = false;
    mChestLeftUpperArmJoint = dynamic_cast<rp3d::BallAndSocketJoint *>(mPhysicsWorld->createJoint(jointInfo2));
    mChestLeftUpperArmJoint->setConeLimitHalfAngle(180.0 * rp3d::PI_RP3D / 180.0);
    mChestLeftUpperArmJoint->enableConeLimit(true);

    // --------------- Create the joint between left upper arm and left lower arm  --------------- //

    // Create the joint info object
    body1 = mLeftUpperArmBone->GetPhysicsObject()->getRigidBody();
    body2 = mLeftLowerArmBone->GetPhysicsObject()->getRigidBody();
    rp3d::Vector3 joint2WorldAnchor =
            (body1->getTransform().getPosition() + body2->getTransform().getPosition()) * 0.5f;
    rp3d::Vector3 joint2WorldAxis(0, 0, 1);
    rp3d::HingeJointInfo jointInfo3(body1, body2, joint2WorldAnchor,
                                    joint2WorldAxis);
    jointInfo3.isCollisionEnabled = false;
    mLeftUpperLeftLowerArmJoint = dynamic_cast<rp3d::HingeJoint *>(mPhysicsWorld->createJoint(jointInfo3));
    mLeftUpperLeftLowerArmJoint->enableLimit(true);
    mLeftUpperLeftLowerArmJoint->setMinAngleLimit(0.0 * rp3d::PI_RP3D / 180.0);
    mLeftUpperLeftLowerArmJoint->setMaxAngleLimit(340.0 * rp3d::PI_RP3D / 180.0);

    // --------------- Create the joint between chest and waist  --------------- //

    // Create the joint info object
    body1 = mChestBone->GetPhysicsObject()->getRigidBody();
    body2 = mWaistBone->GetPhysicsObject()->getRigidBody();
    rp3d::Vector3 jointChestWaistWorldAnchor =
            (body1->getTransform().getPosition() + body2->getTransform().getPosition()) * 0.5f;
    rp3d::FixedJointInfo jointChestWaistInfo(body1, body2, jointChestWaistWorldAnchor);
    jointChestWaistInfo.isCollisionEnabled = false;
    mChestWaistJoint = dynamic_cast<rp3d::FixedJoint *>(mPhysicsWorld->createJoint(jointChestWaistInfo));

    // --------------- Create the joint between waist and hips  --------------- //

    // Create the joint info object
    body1 = mWaistBone->GetPhysicsObject()->getRigidBody();
    body2 = mHipBone->GetPhysicsObject()->getRigidBody();
    rp3d::Vector3 jointWaistHipsWorldAnchor =
            (body1->getTransform().getPosition() + body2->getTransform().getPosition()) * 0.5f;
    rp3d::FixedJointInfo jointWaistHipsInfo(body1, body2, jointWaistHipsWorldAnchor);
    jointWaistHipsInfo.isCollisionEnabled = false;
    mWaistHipsJoint = dynamic_cast<rp3d::FixedJoint *>(mPhysicsWorld->createJoint(jointWaistHipsInfo));

    // --------------- Create the joint between hip and left upper leg --------------- //

    // Create the joint info object
    body1 = mHipBone->GetPhysicsObject()->getRigidBody();
    body2 = mLeftUpperLegBone->GetPhysicsObject()->getRigidBody();
    rp3d::BallAndSocketJointInfo jointInfo4(body1, body2,
                                            mHipBone->GetPosition() + rp3d::Vector3(0.8, 0, 0));
    jointInfo4.isCollisionEnabled = false;
    mHipLeftUpperLegJoint = dynamic_cast<rp3d::BallAndSocketJoint *>(mPhysicsWorld->createJoint(jointInfo4));
    mHipLeftUpperLegJoint->setConeLimitHalfAngle(80.0 * rp3d::PI_RP3D / 180.0);
    mHipLeftUpperLegJoint->enableConeLimit(true);

    // --------------- Create the joint between left upper leg and left lower leg  --------------- //

    // Create the joint info object
    body1 = mLeftUpperLegBone->GetPhysicsObject()->getRigidBody();
    body2 = mLeftLowerLegBone->GetPhysicsObject()->getRigidBody();
    rp3d::Vector3 joint5WorldAnchor =
            (body1->getTransform().getPosition() + body2->getTransform().getPosition()) * 0.5f;
    rp3d::Vector3 joint5WorldAxis(1, 0, 0);
    const rp3d::decimal joint5MinAngle = 0.0 * rp3d::PI_RP3D / 180.0;
    const rp3d::decimal joint5MaxAngle = 140.0 * rp3d::PI_RP3D / 180.0;
    rp3d::HingeJointInfo jointInfo5(body1, body2, joint5WorldAnchor,
                                    joint5WorldAxis, joint5MinAngle, joint5MaxAngle);
    jointInfo5.isCollisionEnabled = false;
    mLeftUpperLeftLowerLegJoint = dynamic_cast<rp3d::HingeJoint *>(mPhysicsWorld->createJoint(jointInfo5));

    // --------------- Create the joint between chest and right upper arm --------------- //

    // Create the joint info object
    body1 = mChestBone->GetPhysicsObject()->getRigidBody();
    body2 = mRightUpperArmBone->GetPhysicsObject()->getRigidBody();
    rp3d::BallAndSocketJointInfo jointInfo6(body1, body2,
                                            mRightUpperArmBone->GetPosition() + rp3d::Vector3(1, 0, 0));
    jointInfo6.isCollisionEnabled = false;
    mChestRightUpperArmJoint = dynamic_cast<rp3d::BallAndSocketJoint *>(mPhysicsWorld->createJoint(jointInfo6));
    mChestRightUpperArmJoint->setConeLimitHalfAngle(180.0 * rp3d::PI_RP3D / 180.0);
    mChestRightUpperArmJoint->enableConeLimit(true);

    // --------------- Create the joint between right upper arm and right lower arm  --------------- //

    // Create the joint info object
    body1 = mRightUpperArmBone->GetPhysicsObject()->getRigidBody();
    body2 = mRightLowerArmBone->GetPhysicsObject()->getRigidBody();
    rp3d::Vector3 joint7WorldAnchor =
            (body1->getTransform().getPosition() + body2->getTransform().getPosition()) * 0.5f;
    rp3d::Vector3 joint7WorldAxis(0, 0, 1);
    rp3d::HingeJointInfo jointInfo7(body1, body2, joint7WorldAnchor, joint7WorldAxis);
    jointInfo7.isCollisionEnabled = false;
    mRightUpperRightLowerArmJoint = dynamic_cast<rp3d::HingeJoint *>(mPhysicsWorld->createJoint(jointInfo7));
    mRightUpperRightLowerArmJoint->enableLimit(true);
    mRightUpperRightLowerArmJoint->setMinAngleLimit(0.0 * rp3d::PI_RP3D / 180.0);
    mRightUpperRightLowerArmJoint->setMaxAngleLimit(340.0 * rp3d::PI_RP3D / 180.0);

    // --------------- Create the joint between hips and right upper leg --------------- //

    // Create the joint info object
    body1 = mHipBone->GetPhysicsObject()->getRigidBody();
    body2 = mRightUpperLegBone->GetPhysicsObject()->getRigidBody();
    rp3d::BallAndSocketJointInfo jointInfo8(body1, body2, mHipBone->GetPosition() + rp3d::Vector3(-0.8, 0, 0));
    jointInfo8.isCollisionEnabled = false;
    mHipRightUpperLegJoint = dynamic_cast<rp3d::BallAndSocketJoint *>(mPhysicsWorld->createJoint(jointInfo8));
    mHipRightUpperLegJoint->setConeLimitHalfAngle(80.0 * rp3d::PI_RP3D / 180.0);
    mHipRightUpperLegJoint->enableConeLimit(true);

    // --------------- Create the joint between right upper leg and right lower leg  --------------- //

    // Create the joint info object
    body1 = mRightUpperLegBone->GetPhysicsObject()->getRigidBody();
    body2 = mRightLowerLegBone->GetPhysicsObject()->getRigidBody();
    rp3d::Vector3 joint9WorldAnchor =
            (body1->getTransform().getPosition() + body2->getTransform().getPosition()) * 0.5f;
    rp3d::Vector3 joint9WorldAxis(1, 0, 0);
    const rp3d::decimal joint9MinAngle = 0.0 * rp3d::PI_RP3D / 180.0;
    const rp3d::decimal joint9MaxAngle = 140.0 * rp3d::PI_RP3D / 180.0;
    rp3d::HingeJointInfo jointInfo9(body1, body2, joint9WorldAnchor, joint9WorldAxis, joint9MinAngle, joint9MaxAngle);
    jointInfo9.isCollisionEnabled = false;
    mRightUpperRightLowerLegJoint = dynamic_cast<rp3d::HingeJoint *>(mPhysicsWorld->createJoint(jointInfo9));

}

Skeleton::~Skeleton() {
}


void Skeleton::initBodiesPositions() {
//    mHeadBox->setTransform(rp3d::Transform(mHeadPos, rp3d::Quaternion::identity()));
//    mChestBox->setTransform(rp3d::Transform(mChestPos, rp3d::Quaternion::fromEulerAngles(0, 0, rp3d::PI_RP3D / 2.0)));
//    mWaistBox->setTransform(rp3d::Transform(mWaistPos, rp3d::Quaternion::identity()));
//    mHipBox->setTransform(rp3d::Transform(mHipPos, rp3d::Quaternion::fromEulerAngles(0, 0, rp3d::PI_RP3D / 2.0)));
//    mLeftUpperArmBox->setTransform(rp3d::Transform(mLeftUpperArmPos, rp3d::Quaternion::fromEulerAngles(0, 0, rp3d::PI_RP3D / 2.0)));
//    mLeftLowerArmBox->setTransform(rp3d::Transform(mLeftLowerArmPos, rp3d::Quaternion::fromEulerAngles(0, 0, rp3d::PI_RP3D / 2.0)));
//    mLeftUpperLegBox->setTransform(rp3d::Transform(mLeftUpperLegPos, rp3d::Quaternion::identity()));
//    mLeftLowerLegBox->setTransform(rp3d::Transform(mLeftLowerLegPos, rp3d::Quaternion::identity()));
//    mRightUpperArmBox->setTransform(rp3d::Transform(mRightUpperArmPos, rp3d::Quaternion::fromEulerAngles(0, 0, rp3d::PI_RP3D / 2.0)));
//    mRightLowerArmBox->setTransform(rp3d::Transform(mRightLowerArmPos, rp3d::Quaternion::fromEulerAngles(0, 0, rp3d::PI_RP3D / 2.0)));
//    mRightUpperLegBox->setTransform(rp3d::Transform(mRightUpperLegPos, rp3d::Quaternion::identity()));
//    mRightLowerLegBox->setTransform(rp3d::Transform(mRightLowerLegPos, rp3d::Quaternion::identity()));
}

void
Skeleton::SetLeftUpperLeftLowerArmJointRotation(rp3d::decimal angleX, rp3d::decimal angleY, rp3d::decimal angleZ) {
    mLeftLowerArmBone->GetPhysicsObject()->setTransform(rp3d::Transform(mLeftLowerArmBone->GetPosition(),
                                                                        rp3d::Quaternion::fromEulerAngles(angleX,
                                                                                                          angleY,
                                                                                                          angleZ)));

}

void Skeleton::RotateJoint(Bone *bone, rp3d::decimal angleX, rp3d::decimal angleY, rp3d::decimal angleZ) {
    rp3d::Quaternion quatern = bone->GetPhysicsObject()->getTransform().getOrientation();

    rp3d::Quaternion new_quaternion = rp3d::Quaternion::fromEulerAngles(angleX, angleY, angleZ) * quatern;

    bone->GetPhysicsObject()->setTransform(
            rp3d::Transform(bone->GetPosition(), new_quaternion));

    // Event occur!!!
    bone_transform_changed.fire(bone);
    bone->UpdateChild();
}


Bone *Skeleton::FindBone(rp3d::RigidBody *body) {
    Bone *target = nullptr;
    for (auto &[key, bone]: bones) {
        if (bone->GetPhysicsObject()->getRigidBody() == body) {
            target = bone;
            break;
        }
    }
    return target;
}

Bone *Skeleton::FindBone(const string &name) {
    Bone *target;
    for (auto &[key, bone]: bones) {
        if (key == name) {
            target = bone;
            break;
        }
    }
    return target;
}

