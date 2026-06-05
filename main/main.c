/**
 * @file main.c
 * @brief W5500 Ethernet Basic Example
 *
 * Demonstrates:
 * - Basic initialization with DHCP
 * - Status callbacks
 * - Waiting for connection
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "eth_w5500.h"

static const char *TAG = "eth_example";

// Status change callback
static void on_eth_status(eth_w5500_status_t status, void *ctx)
{
    switch (status) {
        case ETH_W5500_STATUS_STOPPED:
            ESP_LOGI(TAG, "Ethernet stopped");
            break;
        case ETH_W5500_STATUS_STARTED:
            ESP_LOGI(TAG, "Ethernet started (no link)");
            break;
        case ETH_W5500_STATUS_LINK_UP:
            ESP_LOGI(TAG, "Link up - waiting for IP...");
            break;
        case ETH_W5500_STATUS_GOT_IP:
            ESP_LOGI(TAG, "Got IP: %s", eth_w5500_ip_to_str(eth_w5500_get_ip()));
            break;
        case ETH_W5500_STATUS_ERROR:
            ESP_LOGE(TAG, "Ethernet error");
            break;
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "W5500 Ethernet Example");

    // Register callback before init
    eth_w5500_on_status_change(on_eth_status, NULL);

    // Configure W5500
    w5500_driver_config_t cfg = W5500_DRIVER_DEFAULT_CONFIG();

    // Set your pins here
    cfg.spi_mosi = 23;
    cfg.spi_miso = 19;
    cfg.spi_clk = 18;
    cfg.spi_cs = 5;

    // Optional: custom hostname
    cfg.hostname = "my-esp32";

    // Optional: static IP instead of DHCP
    // cfg.use_dhcp = false;
    // cfg.static_ip = ETH_W5500_IP4(192, 168, 1, 100);
    // cfg.static_gateway = ETH_W5500_IP4(192, 168, 1, 1);
    // cfg.static_netmask = ETH_W5500_IP4(255, 255, 255, 0);

    // Initialize
    esp_err_t ret = eth_w5500_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Init failed: %s", esp_err_to_name(ret));
        return;
    }

    // Start Ethernet
    ret = eth_w5500_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Start failed: %s", esp_err_to_name(ret));
        return;
    }

    // Wait for connection (30 second timeout)
    ESP_LOGI(TAG, "Waiting for connection...");
    if (eth_w5500_wait_for_ip(30000)) {
        ESP_LOGI(TAG, "Connected!");

        // Print full info
        eth_w5500_info_t info;
        eth_w5500_get_info(&info);

        ESP_LOGI(TAG, "IP:      %s", eth_w5500_ip_to_str(info.ip_addr));
        ESP_LOGI(TAG, "Gateway: %s", eth_w5500_ip_to_str(info.gateway));
        ESP_LOGI(TAG, "Netmask: %s", eth_w5500_ip_to_str(info.netmask));
        ESP_LOGI(TAG, "DNS:     %s", eth_w5500_ip_to_str(info.dns));
        ESP_LOGI(TAG, "MAC:     %02X:%02X:%02X:%02X:%02X:%02X",
                 info.mac[0], info.mac[1], info.mac[2],
                 info.mac[3], info.mac[4], info.mac[5]);
        ESP_LOGI(TAG, "Speed:   %d Mbps", info.link_speed);
    } else {
        ESP_LOGW(TAG, "Connection timeout");
    }

    // Main loop - monitor status
    while (1) {
        if (eth_w5500_is_connected()) {
            // Do your connected tasks here
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
