#include "behaviortree_cpp/bt_factory.h"

using namespace BT;

/*
 * Sometimes it is convenient to pass additional (static) arguments to a Node.
 * If these parameters are known at comiplation time and they don't change at
 * run-time, input ports are probably overkill.
 *
 * This tutorial demonstrate two possible ways to initialize a custom node with
 * some additional arguments.
 *
 * Action_A will have a different constructor than the default one.
 *
 * Action_B instead implements an init(...) method that must be called at the beginning.
 */



class Action_A: public SyncActionNode
{

public:
    // additional arguments passed to the constructor
    Action_A(const std::string& name, const NodeConfiguration& config,
             int arg1, double arg2, std::string arg3 ):
        SyncActionNode(name, config),
        _arg1(arg1),
        _arg2(arg2),
        _arg3(arg3) {}

    NodeStatus tick() override
    {
        std::cout << "Action_B: " << _arg1 << " / " << _arg2 << " / " << _arg3 << std::endl;
        return NodeStatus::SUCCESS;
    }

    static PortsList providedPorts() { return {}; }

private:
    int _arg1;
    double _arg2;
    std::string _arg3;
};

class Action_B: public SyncActionNode
{

public:
    Action_B(const std::string& name, const NodeConfiguration& config):
        SyncActionNode(name, config) {}

    // we want this method to be called ONCE and BEFORE the first tick()
    void init( int arg1, double arg2, std::string arg3 )
    {
        _arg1 = (arg1);
        _arg2 = (arg2);
        _arg3 = (arg3);
    }

    NodeStatus tick() override
    {
        std::cout << "Action_B: " << _arg1 << " / " << _arg2 << " / " << _arg3 << std::endl;
        return NodeStatus::SUCCESS;
    }

    static PortsList providedPorts() { return {}; }

private:
    int _arg1;
    double _arg2;
    std::string _arg3;
};

// Simple tree, just to show the outputs of the respective tick()
static const char* xml_text = R"(

 <root main_tree_to_execute = "MainTree" >

     <BehaviorTree ID="MainTree">
        <Sequence>
            <Action_A/>
            <Action_B/>
        </Sequence>
     </BehaviorTree>

 </root>
 )";

int main()
{
    BehaviorTreeFactory factory;

    // A node builder is nothing more than a funtion pointer to create a std::unique_ptr<TreeNode>
    // using lambdas or std::bind we an easily "inject" additional arguments.
    NodeBuilder builder_A = [](const std::string& name, const NodeConfiguration& config)
    {
        return std::unique_ptr<Action_A>( new Action_A(name, config, 42, 3.14, "hello world") );
    };

    // You may create this by hand, but in this case you have a convenient helper function
    // called BehaviorTreeFactory::buildManifest
    TreeNodeManifest manifest_A = BehaviorTreeFactory::buildManifest<Action_A>("Action_A");

    // BehaviorTreeFactory::registerBuilder is the more general way to register a custom node.
    // Not the most user friendly, but defenetively the most flexible one.
    factory.registerBuilder( manifest_A, builder_A);

    // The regitration of  Action_B is done as usual, but we still need to call Action_B::init()
    factory.registerNodeType<Action_B>( "Action_B" );

    auto tree = factory.createTreeFromText(xml_text);

    // Iterate through all the nodes and call init if it is an Action_B
    for( auto& node: tree.nodes )
    {
        if( auto action_B_node = dynamic_cast<Action_B*>( node.get() ))
        {
            action_B_node->init( 69, 9.99, "interesting_value" );
        }
    }

    tree.root_node->executeTick();

    /* Expected output:

        Action_B: 42 / 3.14 / hello world
        Action_B: 69 / 9.99 / interesting_value
    */
    return 0;
}
