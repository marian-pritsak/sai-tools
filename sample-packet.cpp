#include <iostream>
#include <vector>
#include <string.h>

extern "C" {
#include <sai.h>
}

sai_object_id_t g_switch_id;
sai_switch_api_t *sai_switch_api;
sai_port_api_t *sai_port_api;
sai_hostif_api_t *sai_hostif_api;
sai_samplepacket_api_t *sai_samplepacket_api;

const char* profile_get_value(
        _In_ sai_switch_profile_id_t profile_id,
        _In_ const char *variable)
{
    return "/tmp/sai.profile";
}

int profile_get_next_value(
        _In_ sai_switch_profile_id_t profile_id,
        _Out_ const char **variable,
        _Out_ const char **value)
{
    return -1;
}

service_method_table_t test_services = {
        profile_get_value,
            profile_get_next_value
};

sai_object_id_t sai_get_port_id_by_front_port(uint32_t hw_port)
{
    sai_object_id_t new_objlist[32];
    sai_attribute_t sai_attr;
    sai_attr.id = SAI_SWITCH_ATTR_PORT_NUMBER;
    uint32_t max_ports = 32; //sai_attr.value.u32;

    sai_attr.id = SAI_SWITCH_ATTR_PORT_LIST;
    sai_attr.value.objlist.count = max_ports;
    sai_attr.value.objlist.list = &new_objlist[0];
    sai_switch_api->get_switch_attribute(g_switch_id, 1, &sai_attr);

    sai_attribute_t hw_lane_list_attr;

    for (unsigned int i = 0; i < max_ports; i++)
    {
        uint32_t hw_port_list[4];
        hw_lane_list_attr.id = SAI_PORT_ATTR_HW_LANE_LIST;
        hw_lane_list_attr.value.u32list.list = &hw_port_list[0];
        hw_lane_list_attr.value.u32list.count = 4;
        sai_port_api->get_port_attribute(sai_attr.value.objlist.list[i], 1,
                &hw_lane_list_attr);
        if (hw_port_list[0] == ((hw_port - 1) * 4))
        {
            return sai_attr.value.objlist.list[i];
        }
    }
    return -1;
}

int main(int argc, char **argv)
{
    sai_status_t status = sai_api_initialize(0, &test_services);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to initialize SAI api." << std::endl;
        return 1;
    }

    status = sai_api_query(SAI_API_SWITCH, (void**) &sai_switch_api);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to query switch api" << std::endl;
        return 1;
    }

    status = sai_api_query(SAI_API_PORT, (void**) &sai_port_api);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to query port api" << std::endl;
        return 1;
    }

    sai_object_id_t switch_id;
    sai_attribute_t attr;
    std::vector<sai_attribute_t> attrs;

    attr.id = SAI_SWITCH_ATTR_INIT_SWITCH;
    attr.value.booldata = false;
    attrs.push_back(attr);
    
    status = sai_switch_api->create_switch(&g_switch_id, attrs.size(), attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to connect to SAI." << std::endl;
        return 1;
    }

    status = sai_api_query(SAI_API_HOSTIF, (void**) &sai_hostif_api);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to query hostif api" << std::endl;
        return 1;
    }

    attrs.clear();
    sai_object_id_t host_intfs_id;
    sai_object_id_t port_id = sai_get_port_id_by_front_port(1);

    attr.id = SAI_HOSTIF_ATTR_TYPE;
    attr.value.s32 = SAI_HOSTIF_TYPE_NETDEV;
    attrs.push_back(attr);

    attr.id = SAI_HOSTIF_ATTR_OBJ_ID;
    attr.value.oid = port_id;
    attrs.push_back(attr);

    attr.id = SAI_HOSTIF_ATTR_OPER_STATUS;
    attr.value.booldata = true;
    attrs.push_back(attr);

    attr.id = SAI_HOSTIF_ATTR_NAME;
    strncpy((char *)&attr.value.chardata, "samplepacket", strlen("samplepacket"));
    attrs.push_back(attr);

    status = sai_hostif_api->create_hostif(&host_intfs_id, g_switch_id, (uint32_t)attrs.size(), attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create host interface." << std::endl;
        return 1;
    }

    status = sai_api_query(SAI_API_SAMPLEPACKET, (void**) &sai_samplepacket_api);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to query samplepacket api" << std::endl;
        return 1;
    }

    attrs.clear();
    sai_object_id_t sample_packet_id;

    attr.id = SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    attr.value.u32 = 10;
    attrs.push_back(attr);

    attr.id = SAI_SAMPLEPACKET_ATTR_TYPE;
    attr.value.s32 = SAI_SAMPLEPACKET_TYPE_SLOW_PATH;
    attrs.push_back(attr);

    attr.id = SAI_SAMPLEPACKET_ATTR_MODE;
    attr.value.s32 = SAI_SAMPLEPACKET_MODE_EXCLUSIVE;
    attrs.push_back(attr);

    status = sai_samplepacket_api->create_samplepacket(&sample_packet_id,
                                                       g_switch_id,
                                                       (uint32_t)attrs.size(),
                                                       attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create samplepacket, status: " << status << std::endl;
        return 1;
    }

    attr.id = SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE;
    attr.value.oid=sample_packet_id;
    status = sai_port_api->set_port_attribute(port_id, &attr);
    if (status != SAI_STATUS_SUCCESS){
        std::cout << "Failed to bind packet sampler to port, rv: " << status << std::endl;
        return 1;
    }

    std::cout << "Done." << std::endl;

    return 0;
}
