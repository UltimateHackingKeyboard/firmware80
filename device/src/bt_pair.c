#include "bt_pair.h"
#include "nus_client.h"
#include <stdint.h>
#include <zephyr/settings/settings.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/addr.h>
#include "bt_conn.h"
#include "bt_scan.h"
#include "legacy/event_scheduler.h"
#include "zephyr/kernel.h"

bool BtPair_LastPairingSucceeded = true;
bool BtPair_OobPairingInProgress = false;

static bool initialized = false;
static struct bt_le_oob oobRemote;
static struct bt_le_oob oobLocal;

struct bt_le_oob* BtPair_GetLocalOob() {
    EventScheduler_Reschedule(k_uptime_get_32() + PAIRING_TIMEOUT, EventSchedulerEvent_EndBtPairing, "Oob pairing timeout.");
    BtPair_OobPairingInProgress = true;
    if (!initialized) {
        int err = bt_le_oob_get_local(BT_ID_DEFAULT, &oobLocal);
        if (err) {
            printk("Failed to get local OOB data (err %d)\n", err);
            return NULL;
        }
        initialized = true;
    }
    return &oobLocal;
}

struct bt_le_oob* BtPair_GetRemoteOob() {
    EventScheduler_Reschedule(k_uptime_get_32() + PAIRING_TIMEOUT, EventSchedulerEvent_EndBtPairing, "Oob pairing timeout.");
    BtPair_OobPairingInProgress = true;
    return &oobRemote;
}

void BtPair_PairCentral() {
    printk ("Scanning for pairable device\n");
    settings_load();
    bt_le_oob_set_sc_flag(true);
    scan_reload_filters();
}

static void bt_foreach_conn_cb(struct bt_conn *conn, void *user_data) {
    bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    // gpt says you should unref here. Don't believe it!
}

static void disconnectAll() {
    bt_conn_foreach(BT_CONN_TYPE_LE, bt_foreach_conn_cb, NULL);
}

void BtPair_PairPeripheral() {
    printk ("Advertising as pairable device\n");
    settings_load();

    bt_le_oob_set_sc_flag(true);

    disconnectAll();
}

void BtPair_EndPairing(bool success, const char* msg) {
    printk("Pairing ended, success = %d: %s\n", success, msg);
    initialized = false;
    BtPair_LastPairingSucceeded = success;
    BtPair_OobPairingInProgress = false;
    bt_le_oob_set_sc_flag(false);
    EventScheduler_Unschedule(EventSchedulerEvent_EndBtPairing);
    scan_reload_filters();
}

struct delete_args_t {
    bool all;
    const bt_addr_le_t* addr;
};

static void bt_foreach_bond_cb_delete(const struct bt_bond_info *info, void *user_data)
{
    int err;

    struct bt_conn* conn;

    struct delete_args_t* args = (struct delete_args_t*)user_data;

    if (!args->all && bt_addr_le_cmp(args->addr, &info->addr) != 0) {
        char addr[32];
        bt_addr_le_to_str(&info->addr, addr, sizeof(addr));
        printk("Not deleting bond for %s\n", addr);
        return;
    }

    if (bt_addr_le_cmp(&Peers[PeerIdLeft].addr, &info->addr) == 0) {
        settings_delete("uhk/addr/left");
    }
    if (bt_addr_le_cmp(&Peers[PeerIdRight].addr, &info->addr) == 0) {
        settings_delete("uhk/addr/right");
    }
    if (bt_addr_le_cmp(&Peers[PeerIdDongle].addr, &info->addr) == 0) {
        settings_delete("uhk/addr/dongle");
    }

    // Get the connection object if the device is currently connected
    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, &info->addr);

    // Unpair the device
    err = bt_unpair(BT_ID_DEFAULT, &info->addr);
    if (err) {
        printk("Failed to unpair (err %d)\n", err);
    } else {
        char addr[32];
        bt_addr_le_to_str(&info->addr, addr, sizeof(addr));
        printk("Unpaired device %s\n", addr);
    }

    // If the device was connected, release the connection object
    if (conn) {
        bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
        bt_conn_unref(conn);
    }
}

void BtPair_Unpair(const bt_addr_le_t addr) {
    bool deleteAll = true;

    for (uint8_t i = 0; i < BLE_ADDR_LEN; i++) {
        if (addr.a.val[i] != 0) {
            deleteAll = false;
            break;
        }
    }

    if (deleteAll) {
        printk("Deleting all bonds!\n");
        settings_delete("uhk/addr/left");
        settings_delete("uhk/addr/right");
        settings_delete("uhk/addr/dongle");
    }

    struct delete_args_t args = {
        .all = deleteAll,
        .addr = &addr
    };

    // Iterate through all stored bonds
    bt_foreach_bond(BT_ID_DEFAULT, bt_foreach_bond_cb_delete, (void*)&args);
}

struct check_bonded_device_args_t {
    const bt_addr_le_t* addr;
    bool* bonded;
};

void checkBondedDevice(const struct bt_bond_info *info, void *user_data) {
    struct check_bonded_device_args_t* args = (struct check_bonded_device_args_t*)user_data;
    char addr[32];
    bt_addr_le_to_str(&info->addr, addr, sizeof(addr));
    char ref[32];
    bt_addr_le_to_str(args->addr, ref, sizeof(ref));
    if (bt_addr_le_cmp(&info->addr, args->addr) == 0) {
        *args->bonded = true;
        printk("Device %s is bonded, ref %s\n", addr, ref);
    }
};

bool BtPair_IsDeviceBonded(const bt_addr_le_t *addr)
{
    bool bonded = false;

    struct check_bonded_device_args_t args = {
        .addr = addr,
        .bonded = &bonded
    };

    // Iterate over all bonded devices
    bt_foreach_bond(BT_ID_DEFAULT, checkBondedDevice, (void*)&args);

    return bonded;
}