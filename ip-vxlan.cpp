#include <iostream>
#include <vector>
#include <string.h>

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

    status = sai_api_query(SAI_API_ROUTER_INTERFACE, (void**) &sai_router_intfs_api);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to query hostif api" << std::endl;
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

    attrs.clear();
    sai_object_id_t overlayIfId;
    sai_object_id_t underlayIfId;

    attr.id = SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID;
    attr.value.oid = gVirtualRouterId;
    attrs.push_back(attr);

    attr.id = SAI_ROUTER_INTERFACE_ATTR_TYPE;
    attr.value.s32 = SAI_ROUTER_INTERFACE_TYPE_LOOPBACK;
    attrs.push_back(attr);

    status = sai_router_intfs_api->create_router_interface(&overlayIfId, g_switch_id, (uint32_t)attrs.size(), attrs.data());

    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create LB interface" << std::endl;
        return 1;
    }

    status = sai_router_intfs_api->create_router_interface(&underlayIfId, g_switch_id, (uint32_t)attrs.size(), attrs.data());

    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create LB interface" << std::endl;
        return 1;
    }

    attrs.clear();
    attr.id = SAI_TUNNEL_ATTR_TYPE;
    attr.value.s32 = SAI_TUNNEL_TYPE_IPINIP;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_ATTR_OVERLAY_INTERFACE;
    attr.value.oid = overlayIfId;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE;
    attr.value.oid = underlayIfId;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_ATTR_ENCAP_SRC_IP;
    attr.value.ipaddr.addr.ip4 = 0x01000001;
    attr.value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_ATTR_DECAP_ECN_MODE;
    attr.value.s32 = SAI_TUNNEL_DECAP_ECN_MODE_COPY_FROM_OUTER;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_ATTR_DECAP_TTL_MODE;
    attr.value.s32 = SAI_TUNNEL_TTL_MODE_PIPE_MODEL;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_ATTR_DECAP_DSCP_MODE;
    attr.value.s32 = SAI_TUNNEL_DSCP_MODE_UNIFORM_MODEL;
    attrs.push_back(attr);

    sai_object_id_t gre_tunnel_id;
    status = sai_tunnel_api->create_tunnel(&gre_tunnel_id, g_switch_id, (uint32_t)attrs.size(), attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create tunnel" << std::endl;
        return 1;
    }

    attrs.clear();
    attr.id = SAI_NEXT_HOP_ATTR_TYPE;
    attr.value.s32 = SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP;
    attrs.push_back(attr);

    attr.id = SAI_NEXT_HOP_ATTR_IP;
    attr.value.ipaddr.addr.ip4 = 0x02000001;
    attr.value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attrs.push_back(attr);

    attr.id = SAI_NEXT_HOP_ATTR_TUNNEL_ID;
    attr.value.oid = gre_tunnel_id;
    attrs.push_back(attr);

    sai_object_id_t next_hop_id;
    sai_next_hop_api->create_next_hop(&next_hop_id, g_switch_id,
                                            static_cast<uint32_t>(attrs.size()),
                                            attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create next hop" << std::endl;
        return 1;
    }

    sai_route_entry_t route_entry;
    route_entry.vr_id = gVirtualRouterId;
    route_entry.switch_id = g_switch_id;
    route_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    route_entry.destination.addr.ip4 = 0x03000001;
    route_entry.destination.mask.ip4 = 0xffffffff;

    sai_attribute_t route_attr;

    route_attr.id = SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID;
    route_attr.value.oid = next_hop_id;

    status = sai_route_api->create_route_entry(&route_entry, 1, &route_attr);
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create route" << std::endl;
        return 1;
    }

    sai_object_id_t bridge_id;

    attrs.clear();
    attr.id = SAI_BRIDGE_ATTR_TYPE;
    attr.value.s32 = SAI_BRIDGE_TYPE_1D;
    attrs.push_back(attr);

    status = sai_bridge_api->create_bridge(
            &bridge_id,
            g_switch_id,
            (uint32_t)attrs.size(),
            attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create bridge" << std::endl;
        return 1;
    }

    sai_object_id_t rif_id;
    attrs.clear();
    attr.id = SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID;
    attr.value.oid = gVirtualRouterId;
    attrs.push_back(attr);

    //uint8_t rmac[] = { 0x50, 0x6b, 0x4b, 0x8f, 0xce, 0x40 };
    uint8_t rmac[] = { 0x24, 0x8a, 0x07, 0x02, 0x9b, 0x00 };
    attr.id = SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS;
    memcpy(attr.value.mac, rmac, sizeof(sai_mac_t));
    attrs.push_back(attr);

    attr.id = SAI_ROUTER_INTERFACE_ATTR_TYPE;
    attr.value.s32 = SAI_ROUTER_INTERFACE_TYPE_BRIDGE;
    attrs.push_back(attr);

    attr.id = SAI_ROUTER_INTERFACE_ATTR_BRIDGE_ID;
    attr.value.oid = bridge_id;
    attrs.push_back(attr);

    status = sai_router_intfs_api->create_router_interface(
            &rif_id,
            g_switch_id,
            (uint32_t)attrs.size(),
            attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create bridge rif" << std::endl;
        return 1;
    }

    sai_object_id_t bridge_port_rif_id;
    attrs.clear();
    attr.id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr.value.s32 = SAI_BRIDGE_PORT_TYPE_1D_ROUTER;
    attrs.push_back(attr);

    attr.id = SAI_BRIDGE_PORT_ATTR_RIF_ID;
    attr.value.oid = rif_id;
    attrs.push_back(attr);

    attr.id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr.value.oid = bridge_id;
    attrs.push_back(attr);

    attr.id = SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE;
    attr.value.s32 = SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DISABLE;
    attrs.push_back(attr);

    status = sai_bridge_api->create_bridge_port(
            &bridge_port_rif_id,
            g_switch_id,
            (uint32_t)attrs.size(),
            attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create bridge port" << std::endl;
        return 1;
    }

    sai_object_id_t tunnel_map_id;
    attrs.clear();

    attr.id = SAI_TUNNEL_MAP_ATTR_TYPE;
    attr.value.s32 = SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF;
    attrs.push_back(attr);
    
    status = sai_tunnel_api->create_tunnel_map(
                                &tunnel_map_id,
                                g_switch_id,
                                static_cast<uint32_t>(attrs.size()),
                                attrs.data()
                          );
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create tunnel map" << std::endl;
        return 1;
    }


    sai_object_id_t tunnel_map_entry_id;
    attrs.clear();

    attr.id = SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE;
    attr.value.s32 = SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP;
    attr.value.oid = tunnel_map_id;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_VALUE;
    attr.value.oid = bridge_id;
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
    sai_object_id_t underlay_rif;

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

    sai_object_id_t vxlan_tunnel_id;
    attrs.clear();

    attr.id = SAI_TUNNEL_ATTR_TYPE;
    attr.value.s32 = SAI_TUNNEL_TYPE_VXLAN;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE;
    attr.value.oid = underlay_rif;
    attrs.push_back(attr);

    sai_object_id_t decap_list[] = { tunnel_map_id };
    attr.id = SAI_TUNNEL_ATTR_DECAP_MAPPERS;
    attr.value.objlist.count = 1;
    attr.value.objlist.list = decap_list;
    attrs.push_back(attr);

    attr.id = SAI_TUNNEL_ATTR_ENCAP_SRC_IP;
    attr.value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr.value.ipaddr.addr.ip4 = 0x05050505;
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


    sai_object_id_t bridge_port_tunn_id;
    attrs.clear();

    attr.id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr.value.s32 = SAI_BRIDGE_PORT_TYPE_TUNNEL;
    attrs.push_back(attr);

    attr.id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr.value.oid = bridge_id;
    attrs.push_back(attr);

    attr.id = SAI_BRIDGE_PORT_ATTR_ADMIN_STATE;
    attr.value.booldata = true;
    attrs.push_back(attr);

    attr.id = SAI_BRIDGE_PORT_ATTR_TUNNEL_ID;
    attr.value.oid = vxlan_tunnel_id;
    attrs.push_back(attr);

    attr.id = SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE;
    attr.value.s32 = SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DISABLE;
    attrs.push_back(attr);

    status = sai_bridge_api->create_bridge_port(
            &bridge_port_tunn_id,
            g_switch_id,
            (uint32_t)attrs.size(),
            attrs.data());
    if (status != SAI_STATUS_SUCCESS)
    {
        std::cout << "Failed to create bridge port for tunnel" << std::endl;
        return 1;
    }


    std::cout << "Done." << std::endl;
    return 0;
}
