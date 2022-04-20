
# api.emerald-ems.com.au documentation

#### Login and retrieve authentication (bearer token)
##### Request
```http
POST https://api.emerald-ems.com.au/api/v1/customer/sign-in HTTP/1.1
Content-Type: application/json; charset=UTF-8
Content-Length: 123
Host: api.emerald-ems.com.au
Connection: Keep-Alive
Accept-Encoding: gzip
User-Agent: ok

Body: {
    "app_version": "1.2.1",
    "device_name": "Xiaomi MIX Alpha",
    "device_os_version": "12",
    "device_token": "",
    "device_type": "android",
    "email": "my_super_secret_email@email.com",
    "passcode": null,
    "password": "my_super_secret_pass"
}
```
##### Response
```json
{
    "code": 200,
    "message": "Login successful.",
    "info": {
        "id": "73e64cf7-96bb-4c77-ad05-ca351fcb62ee",
        "first_name": "SUPER",
        "last_name": "SECRET",
        "email": "my_super_secret_email@email.com",
        "phone_no": "0412345678",
        "premium": 1,
        "agent_id": "2068b28c-1b00-47d4-b6b2-923f52746bea",
        "social_type": null,
        "freestanding": 1,
        "is_tc_accepted": 1,
        "status": "Completed",
        "deleted_by": null,
        "zendesk_chat": 0,
        "feedback_status": true,
        "weekly_report": false,
        "auto_sync": false,
        "created_at": "2020-05-12 18:20:04",
        "payment": null,
        "full_name": "SUPER SECRET",
        "background_sync_count": 0,
        "pending_rating": null,
        "feedback_form": false
    },
    "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c"
}
```
Use the received token (decodable at jwt.io) for the rest of the API requests

Token doesn't include an expiration but seems to last 24 hours

---

#### Refresh token (retrieves a new bearer token)
##### Request
```http
POST https://api.emerald-ems.com.au/api/v1/customer/token-refresh HTTP/1.1
Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c
Content-Type: application/json; charset=UTF-8
Content-Length: 321
Host: api.emerald-ems.com.au
Connection: Keep-Alive
Accept-Encoding: gzip
User-Agent: ok

Body: {
    "app_version": "1.2.1",
    "device_name": "Xiaomi MIX Alpha",
    "device_os_version": "12",
    "device_type": "android",
    "background_sync_count": 0
}
```
##### Response
```json
{
    "code": 200,
    "message": "Token refreshed successfully.",
    "info": {
        "id": "73e64cf7-96bb-4c77-ad05-ca351fcb62ee",
        "first_name": "SUPER",
        "last_name": "SECRET",
        "email": "my_super_secret_email@email.com",
        "phone_no": "0412345678",
        "premium": 1,
        "agent_id": "2068b28c-1b00-47d4-b6b2-923f52746bea",
        "social_type": null,
        "freestanding": 1,
        "is_tc_accepted": 1,
        "status": "Completed",
        "deleted_by": null,
        "zendesk_chat": 0,
        "feedback_status": true,
        "weekly_report": false,
        "auto_sync": false,
        "created_at": "2020-05-12 18:20:04",
        "payment": null,
        "full_name": "SUPER SECRET",
        "background_sync_count": 0,
        "pending_rating": null,
        "feedback_form": false
    },
    "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5a"
}
```

---

#### Retrieve property list
##### Request
```http
GET https://api.emerald-ems.com.au/api/v1/customer/property/list HTTP/1.1
Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c
Host: api.emerald-ems.com.au
Connection: Keep-Alive
Accept-Encoding: gzip
User-Agent: ok
```
##### Response
```json
{
    "code": 200,
    "message": "Property list",
    "info": {
        "property": [
            {
                "id": "1bf21e83-cf1e-4cce-90e6-642d0294eecd",
                "customer_id": "73e64cf7-96bb-4c77-ad05-ca351fcb62ee",
                "property_name": "123 SUPER SECRET STREET",
                "house_no": null,
                "unit_number": null,
                "street_type": "STREET",
                "street": "SUPER SECRET",
                "city": "SECRET SUBURB",
                "unit_type": null,
                "level_type": null,
                "level_number": null,
                "street_number": "123",
                "street_suffix": null,
                "state": "WA",
                "postal_code": "1234",
                "premium": "No",
                "is_structure_deleted": 0,
                "is_structure_updated": 0,
                "created_at": "2020-05-12 18:20:04",
                "updated_at": "2020-05-12 18:22:32",
                "property_type": "Self",
                "tariff_structure": [
                    {
                        "calculated_supply_charge": 0.6121,
                        "calculated_unit_charge": 0.2035,
                        "company_id": "895eb68a-d4f7-40e4-9cab-33677cdaef73",
                        "discount_percentage": 0,
                        "discount_type": "percent",
                        "electricity_retailer_id": "5f0179e2-7196-47c2-8ff5-6e19f2d550b7",
                        "end_date": null,
                        "gst_include": 1,
                        "impluse_rate": null,
                        "is_flat_rate": 1,
                        "same_as_weekdays": null,
                        "property_id": "1bf21e83-cf1e-4cce-90e6-642d0294eecd",
                        "start_date": "2020-04-11",
                        "supply_charge": 0.6121,
                        "supply_charge_discount": 0,
                        "tariff_seasonal": null,
                        "tariff_plan": null,
                        "tariff_structure_id": "b1fd7135-225b-409c-8f98-fef6e65dcb1d",
                        "type": null,
                        "unit_charge": 0.2035
                    }
                ],
                "budget": null,
                "devices": [
                    {
                        "id": "a1fe40e9-b8be-4081-ad63-82d10ef3e022",
                        "serial_number": "2108123123",
                        "device_identifier": null,
                        "property_id": "1bf21e83-cf1e-4cce-90e6-642d0294eecd",
                        "device_type_id": null,
                        "device_category": "Electricity Advisor",
                        "pairing_code": null,
                        "brand": "Emerald",
                        "model": "Electrcity Advisor",
                        "device_type": "IHD",
                        "firmware_version": "2.1.1",
                        "device_mac_address": "30:1B:97:00:00:00",
                        "device_uid": null,
                        "device_name": "EIAdv 2108123123",
                        "installer_id": null,
                        "agency_id": null,
                        "customer_id": "73e64cf7-96bb-4c77-ad05-ca351fcb62ee",
                        "parent_agency_id": null,
                        "passcode": null,
                        "installation_date": "2024-02-25",
                        "NMI": "62824023840",
                        "impulse_rate": 1000,
                        "device_status": "Active",
                        "delete_reason": null,
                        "first_report": 0,
                        "installation_type": "advance",
                        "replace_id": null,
                        "premium": null,
                        "premium_validity": null,
                        "downloaded_at": null,
                        "is_china_newyear_issue_fixed": 1,
                        "force_sync_date": null,
                        "date_error_exists": 0,
                        "ihd_reset_date": null,
                        "created_at": "2024-02-25 09:22:59",
                        "updated_at": "2024-02-25 11:21:28",
                        "replaced_at": null,
                        "nmi": "62824023840",
                        "impulse_rate_type": "imp/kWh",
                        "impulse_rates": [
                            {
                                "start_date": "2024-02-25",
                                "end_date": null,
                                "impulse_rate": 1000,
                                "impulse_rate_type": "imp/kWh"
                            }
                        ],
                        "ota_update": null
                    }
                ]
            }
        ],
        "shared_property": [],
        "badge_count": 0
    }
}
```
Use `info.property[0].devices.id` to retrieve device energy data

---

#### Retrieve energy data of device (by date range)
##### Request
```http
GET https://api.emerald-ems.com.au/api/v1/customer/device/get-by-date/flashes-data?device_id=a1fe40e9-b8be-4081-ad63-82d10ef3e022&start_date=2024-03-10&end_date=2024-03-12 HTTP/1.1
Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c
Host: api.emerald-ems.com.au
Connection: Keep-Alive
Accept-Encoding: gzip
User-Agent: ok
```
##### Response
```json
{
    "code": 200,
    "message": "Device Flashes data shown successfully.",
    "info": {
        "id": "a1fe40e9-b8be-4081-ad63-82d10ef3e022",
        "daily_trend": -80,
        "monthly_trend": 0,
        "synced_timestamp": 1642129123099,
        "average_daily_spend": 1.55,
        "is_newyear_issue_fixed": true,
        "is_china_newyear_issue_fixed": true,
        "force_sync_date": null,
        "date_error_exists": false,
        "ihd_reset_date": null,
        "device_type": "android",
        "daily_consumptions": [
            {
                "actual_consumptions": null,
                "date_string": "2024-03-10",
                "hourly_consumptions": [
                    {
                        "cost": 0.07234892314891234,
                        "hour_string": "00:00",
                        "id": "fe72765a-729b-415f-b93f-9abb6f6bbe89",
                        "is_complete": true,
                        "kwh": 0.4123921349124391,
                        "loadTypeKey": null,
                        "number_of_flashes": 1634,
                        "rate": null,
                        "unit_charge": null
                    },
                    {
                        "cost": 0.02393291239123932,
                        "hour_string": "01:00",
                        "id": "7207963a-9a4c-4859-ae9a-f6a798a73b1a",
                        "is_complete": true,
                        "kwh": 0.23123123999999999,
                        "loadTypeKey": null,
                        "number_of_flashes": 1032,
                        "rate": null,
                        "unit_charge": null
                    },

                    {
                        "cost": 0,
                        "hour_string": "23:00",
                        "id": "e1ed4172-2f23-435f-97bd-4a18b0d6ff4e",
                        "is_complete": false,
                        "kwh": 0,
                        "loadTypeKey": null,
                        "number_of_flashes": 0,
                        "rate": null,
                        "unit_charge": null
                    }
                ],
                "id": "88f1a6d9-a3d9-40a2-bc15-ba5c506a04fe",
                "is_complete": false,
                "ten_minute_consumptions": [
                    {
                        "cost": 0.02128123099,
                        "hideen": false,
                        "id": "8520508d-a3c6-40b8-8497-75ffecdc2832",
                        "kwh": 0.06234923482314982,
                        "loadTypeKey": null,
                        "number_of_flashes": 311,
                        "rate": null,
                        "time_string": "00:00"
                    },
                    {
                        "cost": 0.02128123099,
                        "hideen": false,
                        "id": "86a03aad-9f2d-45f9-a159-b756dcbeb86e",
                        "kwh": 0.06234923482314982,
                        "loadTypeKey": null,
                        "number_of_flashes": 311,
                        "rate": null,
                        "time_string": "00:10"
                    },
                    
                    {
                        "cost": 0,
                        "hideen": false,
                        "id": "6739cece-e507-4319-b12f-d75d2cfb442d",
                        "kwh": 0,
                        "loadTypeKey": null,
                        "number_of_flashes": 0,
                        "rate": null,
                        "time_string": "23:40"
                    },
                    {
                        "cost": 0,
                        "hideen": false,
                        "id": "85eeb63b-2136-4ace-8c17-c3105bb4500f",
                        "kwh": 0,
                        "loadTypeKey": null,
                        "number_of_flashes": 0,
                        "rate": null,
                        "time_string": "23:50"
                    }
                ],
                "total_consumption_of_day": 2038,
                "total_cost_of_day": 0.23912012301230123,
                "total_kwh_of_day": 1.112312349123400
            }
        ]
    }
}
```