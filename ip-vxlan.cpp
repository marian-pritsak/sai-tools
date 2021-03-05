#include <iostream>
#include <vector>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

extern "C" {
#include <sai.h>
}

sai_object_id_t g_switch_id;
sai_switch_api_t *sai_switch_api;
sai_port_api_t *sai_port_api;
sai_tunnel_api_t *sai_tunnel_api;
sai_bridge_api_t *sai_bridge_api;
sai_route_api_t *sai_route_api;
sai_next_hop_api_t *sai_next_hop_api;
sai_next_hop_group_api_t *sai_next_hop_group_api;
sai_hostif_api_t *sai_hostif_api;
sai_samplepacket_api_t *sai_samplepacket_api;
sai_router_interface_api_t* sai_router_intfs_api;

sai_object_id_t gVirtualRouterId;

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

sai_service_method_table_t test_services = {
        profile_get_value,
            profile_get_next_value
};

sai_object_id_t decap_tunnel_map_id;
sai_object_id_t tunnel_map_entry_id;
sai_object_id_t encap_tunnel_map_id;
sai_object_id_t encap_tunnel_map_entry_id;
sai_object_id_t underlay_rif;
sai_object_id_t vxlan_tunnel_id;
sai_object_id_t next_hop_id_10;
sai_object_id_t next_hop_id_11;
sai_object_id_t next_hop_id_12;
sai_object_id_t ecmp_id;
sai_object_id_t ecmp_member_10;
sai_object_id_t ecmp_member_11;
sai_object_id_t ecmp_member_12;
sai_route_entry_t route_entry;

void sighandler(int signum) {
    std::cout << "Route" << std::endl;
    sai_status_t status = sai_route_api->remove_route_entry(&route_entry);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to remove route" << std::endl;
        exit(1);
    }

    std::cout << "members" << std::endl;
    status = sai_next_hop_group_api->remove_next_hop_group_member(ecmp_member_12);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to remove nh group member" << std::endl;
        exit(1);
    }

    status = sai_next_hop_group_api->remove_next_hop_group_member(ecmp_member_11);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to remove nh group member" << std::endl;
        exit(1);
    }

    status = sai_next_hop_group_api->remove_next_hop_group_member(ecmp_member_10);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to remove nh group member" << std::endl;
        exit(1);
    }

    std::cout << "group" << std::endl;
    status = sai_next_hop_group_api->remove_next_hop_group(ecmp_id);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to remove nh group" << std::endl;
        exit(1);
    }

    std::cout << "nhs" << std::endl;
    status = sai_next_hop_api->remove_next_hop(next_hop_id_12);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to remove nh" << std::endl;
        exit(1);
    }

    status = sai_next_hop_api->remove_next_hop(next_hop_id_11);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to remove nh" << std::endl;
        exit(1);
    }

    status = sai_next_hop_api->remove_next_hop(next_hop_id_10);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to remove nh" << std::endl;
        exit(1);
    }

    std::cout << "Done" << std::endl;
    exit(0);
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

    status = sai_api_query(SAI_API_TUNNEL, (void**) &sai_tunnel_api);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to query tunnel api" << std::endl;
        return 1;
    }

    status = sai_api_query(SAI_API_BRIDGE, (void**) &sai_bridge_api);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to query tunnel api" << std::endl;
        return 1;
    }

    status = sai_api_query(SAI_API_ROUTE, (void**) &sai_route_api);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to query route api" << std::endl;
        return 1;
    }

    status = sai_api_query(SAI_API_NEXT_HOP, (void**) &sai_next_hop_api);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to query next_hop api" << std::endl;
        return 1;
    }

    status = sai_api_query(SAI_API_NEXT_HOP_GROUP, (void**) &sai_next_hop_group_api);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to query next_hop api" << std::endl;
        return 1;
    }

    status = sai_api_query(SAI_API_ROUTER_INTERFACE, (void**) &sai_router_intfs_api);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to query rif api" << std::endl;
        return 1;
    }

    sai_log_set(SAI_API_ROUTE, SAI_LOG_LEVEL_DEBUG);
    sai_log_set(SAI_API_NEXT_HOP, SAI_LOG_LEVEL_DEBUG);
    sai_log_set(SAI_API_NEXT_HOP_GROUP, SAI_LOG_LEVEL_DEBUG);

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

    attr.id = SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID;

    status = sai_switch_api->get_switch_attribute(g_switch_id, 1, &attr);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Fail to get switch virtual router ID " << status << std::endl;
        exit(EXIT_FAILURE);
    }

    gVirtualRouterId = attr.value.oid;

    // Tunnel mappers
    attrs.clear();

    attr.id = SAI_TUNNEL_MAP_ATTR_TYPE;
    attr.value.s32 = SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID;
    attrs.push_back(attr);
    
    status = sai_tunnel_api->create_tunnel_map(
                                &decap_tunnel_map_id,
                                g_switch_id,
                                static_cast<uint32_t>(attrs.size()),
                                attrs.data()
                          );
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create tunnel map" << std::endl;
        return 1;
    }


    attrs.clear();

    attr.id = SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE;
    attr.value.s32 = SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP;
    attr.value.oid = decap_tunnel_map_id;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_VALUE;
    attr.value.oid = gVirtualRouterId;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_KEY;
    attr.value.oid = 6;
    attrs.push_back(attr);

    status = sai_tunnel_api->create_tunnel_map_entry(&tunnel_map_entry_id, g_switch_id,
                                            static_cast<uint32_t> (attrs.size()),
                                            attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create tunnel map entry" << std::endl;
        return 1;
    }

    attrs.clear();

    attr.id = SAI_TUNNEL_MAP_ATTR_TYPE;
    attr.value.s32 = SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI;
    attrs.push_back(attr);
    
    status = sai_tunnel_api->create_tunnel_map(
                                &encap_tunnel_map_id,
                                g_switch_id,
                                static_cast<uint32_t>(attrs.size()),
                                attrs.data()
                          );
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create tunnel map" << std::endl;
        return 1;
    }

    attrs.clear();

    attr.id = SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE;
    attr.value.s32 = SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP;
    attr.value.oid = encap_tunnel_map_id;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_KEY;
    attr.value.oid = gVirtualRouterId;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE;
    attr.value.oid = 6;
    attrs.push_back(attr);

    status = sai_tunnel_api->create_tunnel_map_entry(&encap_tunnel_map_entry_id, g_switch_id,
                                            static_cast<uint32_t> (attrs.size()),
                                            attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create tunnel map entry" << std::endl;
        return 1;
    }

    attrs.clear();

    attr.id = SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID;
    attr.value.oid = gVirtualRouterId;
    attrs.push_back(attr);

    attr.id = SAI_ROUTER_INTERFACE_ATTR_TYPE;
    attr.value.s32 = SAI_ROUTER_INTERFACE_TYPE_LOOPBACK;
    attrs.push_back(attr);

    status = sai_router_intfs_api->create_router_interface(&underlay_rif, g_switch_id, (uint32_t)attrs.size(), attrs.data());

    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create LB interface" << std::endl;
        return 1;
    }

    attrs.clear();

    attr.id = SAI_TUNNEL_ATTR_TYPE;
    attr.value.s32 = SAI_TUNNEL_TYPE_VXLAN;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE;
    attr.value.oid = underlay_rif;
    attrs.push_back(attr);

    sai_object_id_t decap_list[] = { decap_tunnel_map_id };
    attr.id = SAI_TUNNEL_ATTR_DECAP_MAPPERS;
    attr.value.objlist.count = 1;
    attr.value.objlist.list = decap_list;
    attrs.push_back(attr);

    sai_object_id_t encap_list[] = { encap_tunnel_map_id };
    attr.id = SAI_TUNNEL_ATTR_ENCAP_MAPPERS;
    attr.value.objlist.count = 1;
    attr.value.objlist.list = encap_list;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_ATTR_ENCAP_SRC_IP;
    attr.value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr.value.ipaddr.addr.ip4 = 0x01010101;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_ATTR_ENCAP_TTL_MODE;
    attr.value.s32 = SAI_TUNNEL_TTL_MODE_PIPE_MODEL;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_ATTR_ENCAP_TTL_VAL;
    attr.value.u8 = 128;
    attrs.push_back(attr);

    status = sai_tunnel_api->create_tunnel(
                                &vxlan_tunnel_id,
                                g_switch_id,
                                static_cast<uint32_t>(attrs.size()),
                                attrs.data()
                          );
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create vxlan tunnel" << std::endl;
        return 1;
    }


    attrs.clear();
    attr.id = SAI_NEXT_HOP_ATTR_TYPE;
    attr.value.s32 = SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP;
    attrs.push_back(attr);

    attr.id = SAI_NEXT_HOP_ATTR_IP;
    attr.value.ipaddr.addr.ip4 = 0x0a010101;
    attr.value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attrs.push_back(attr);

    attr.id = SAI_NEXT_HOP_ATTR_TUNNEL_ID;
    attr.value.oid = vxlan_tunnel_id;
    attrs.push_back(attr);

    status = sai_next_hop_api->create_next_hop(&next_hop_id_10, g_switch_id,
                                            static_cast<uint32_t>(attrs.size()),
                                            attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create next hop" << std::endl;
        return 1;
    }

    attrs.clear();
    attr.id = SAI_NEXT_HOP_ATTR_TYPE;
    attr.value.s32 = SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP;
    attrs.push_back(attr);

    attr.id = SAI_NEXT_HOP_ATTR_IP;
    attr.value.ipaddr.addr.ip4 = 0x0b010101;
    attr.value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attrs.push_back(attr);

    attr.id = SAI_NEXT_HOP_ATTR_TUNNEL_ID;
    attr.value.oid = vxlan_tunnel_id;
    attrs.push_back(attr);

    status = sai_next_hop_api->create_next_hop(&next_hop_id_11, g_switch_id,
                                            static_cast<uint32_t>(attrs.size()),
                                            attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create next hop" << std::endl;
        return 1;
    }

    attrs.clear();
    attr.id = SAI_NEXT_HOP_ATTR_TYPE;
    attr.value.s32 = SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP;
    attrs.push_back(attr);

    attr.id = SAI_NEXT_HOP_ATTR_IP;
    attr.value.ipaddr.addr.ip4 = 0x0c010101;
    attr.value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attrs.push_back(attr);

    attr.id = SAI_NEXT_HOP_ATTR_TUNNEL_ID;
    attr.value.oid = vxlan_tunnel_id;
    attrs.push_back(attr);

    status = sai_next_hop_api->create_next_hop(&next_hop_id_12, g_switch_id,
                                            static_cast<uint32_t>(attrs.size()),
                                            attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create next hop" << std::endl;
        return 1;
    }


    attrs.clear();
    attr.id = SAI_NEXT_HOP_GROUP_ATTR_TYPE;
    attr.value.s32 = SAI_NEXT_HOP_GROUP_TYPE_ECMP;
    attrs.push_back(attr);

    status = sai_next_hop_group_api->create_next_hop_group(&ecmp_id, g_switch_id,
                                            static_cast<uint32_t>(attrs.size()),
                                            attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create next hop group" << std::endl;
        return 1;
    }


    attrs.clear();
    attr.id = SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_GROUP_ID;
    attr.value.oid = ecmp_id;
    attrs.push_back(attr);

    attr.id = SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_ID;
    attr.value.oid = next_hop_id_10;
    attrs.push_back(attr);

    status = sai_next_hop_group_api->create_next_hop_group_member(&ecmp_member_10, g_switch_id,
                                            static_cast<uint32_t>(attrs.size()),
                                            attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create next hop group member" << std::endl;
        return 1;
    }

    attrs.clear();
    attr.id = SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_GROUP_ID;
    attr.value.oid = ecmp_id;
    attrs.push_back(attr);

    attr.id = SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_ID;
    attr.value.oid = next_hop_id_11;
    attrs.push_back(attr);

    status = sai_next_hop_group_api->create_next_hop_group_member(&ecmp_member_11, g_switch_id,
                                            static_cast<uint32_t>(attrs.size()),
                                            attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create next hop group member" << std::endl;
        return 1;
    }

    attrs.clear();
    attr.id = SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_GROUP_ID;
    attr.value.oid = ecmp_id;
    attrs.push_back(attr);

    attr.id = SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_ID;
    attr.value.oid = next_hop_id_12;
    attrs.push_back(attr);

    status = sai_next_hop_group_api->create_next_hop_group_member(&ecmp_member_12, g_switch_id,
                                            static_cast<uint32_t>(attrs.size()),
                                            attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create next hop group member" << std::endl;
        return 1;
    }

    route_entry.vr_id = gVirtualRouterId;
    route_entry.switch_id = g_switch_id;
    route_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    route_entry.destination.addr.ip4 = 0x05050505;
    route_entry.destination.mask.ip4 = 0xffffffff;

    sai_attribute_t route_attr;

    route_attr.id = SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID;
    route_attr.value.oid = ecmp_id;

    status = sai_route_api->create_route_entry(&route_entry, 1, &route_attr);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create route" << std::endl;
        return 1;
    }

    signal(SIGINT, sighandler);

    std::cout << "Done." << std::endl;
    while(1) {
	sleep(1);
    }

    return 0;
}
