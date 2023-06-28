use std::sync::{Mutex};
use lazy_static::lazy_static;
use prometheus::{Encoder, TextEncoder, IntGaugeVec, GaugeVec};
use actix_web::{App, HttpServer, web, HttpResponse, post, Responder, get};
use actix_cors::Cors;

// all input data to be passed as string, embedded device may not parse integer and floats
// hence keeping all of them as string.

// struct to hold parsed data
#[derive(Debug)]
struct DeviceMetric {
    fds_dev_id: String, //Flood Detection System Device Id
    fds_dev_loc: String, //Flood Detection System Devie Location
    temperature: f64,
    humidity: f64,
    heat_index: f64,
    liter_per_hour: f64,
    distance_in_cm: f64,
    water_level: i32
}

#[derive(Debug)]
struct MetricsData {
    data: Mutex<DeviceMetric>
}

lazy_static! {
    static ref ENVIRONMENT_TEMPREATURE: GaugeVec = prometheus::register_gauge_vec!(
        "environment_temperature",
        "environment temperature in celsius from DHT11 Sensor",
        &["fds_device_id","fds_device_location"]
    ).unwrap();

    static ref ENVIRONMENT_HUMIDITY: GaugeVec = prometheus::register_gauge_vec!(
        "environment_humidity",
        "environment humidity in percentage from DHT11 Sensor",
        &["fds_device_id","fds_device_location"]
    ).unwrap();

    static ref HEAT_INDEX: GaugeVec = prometheus::register_gauge_vec!(
        "heat_index",
        "HEAT Index calculated from DHT11 Sensor",
        &["fds_device_id","fds_device_location"]
    ).unwrap();

    static ref LITER_PER_HOUR: GaugeVec = prometheus::register_gauge_vec!(
        "liter_per_hour",
        "Flow Rate obtained from YF-S201 Sensor",
        &["fds_device_id","fds_device_location"]
    ).unwrap();

    static ref DISTANCE_IN_CM: GaugeVec = prometheus::register_gauge_vec!(
        "distance_in_cm",
        "Distance in centemeters obtained from HC-SR04 Ultrasonic Sensor",
        &["fds_device_id","fds_device_location"]
    ).unwrap();

    static ref WATER_LEVEL: IntGaugeVec = prometheus::register_int_gauge_vec!(
        "water_level",
        "Water level status obtained from WATER level sensor which is either 0 or 1, 0 being breached",
        &["fds_device_id","fds_device_location"]
    ).unwrap();
}

//create http web server using actix_web
#[actix_web::main]
async fn main() -> std::io::Result<()>  {

    let metrics_data = web::Data::new(MetricsData {
        data: Mutex::new(DeviceMetric {
            fds_dev_id: String::from("0x00"),
            fds_dev_loc: String::from("Pune-India"),
            temperature: 0.0,
            humidity: 0.0,
            heat_index: 0.0,
            liter_per_hour: 0.0,
            distance_in_cm: 0.0,
            water_level: 0
        })
    });
    
    HttpServer::new(move || {
        
        let cors  = Cors::permissive();
        App::new()
        .wrap(cors)
        .app_data(metrics_data.clone())
        .service(listen)
        .service(get_metrics)
        .service(health)
    })
    .bind(("0.0.0.0",8080))?
    .run()
    .await
}


#[post("/naradamuni/listen")]
async fn listen(raw_payload:  web::Json<serde_json::Value>, data: web::Data<MetricsData>) -> impl Responder {

    let device_metric = DeviceMetric {
        fds_dev_id: raw_payload["fds_dev_id"].as_str().unwrap().to_string(),
        fds_dev_loc: raw_payload["fds_dev_loc"].as_str().unwrap().to_string(),
        temperature: raw_payload["t"].as_str().unwrap().parse().unwrap(),
        humidity: raw_payload["h"].as_str().unwrap().parse().unwrap(),
        heat_index: raw_payload["hic"].as_str().unwrap().parse().unwrap(),
        liter_per_hour: raw_payload["l_hour"].as_str().unwrap().parse().unwrap(),
        distance_in_cm: raw_payload["d_cm"].as_str().unwrap().parse().unwrap(),
        water_level: raw_payload["w_level"].as_str().unwrap().parse().unwrap()
    };

    let mut metrics_data = data.data.lock().unwrap();

    *metrics_data = device_metric;

    println!("{:?}", metrics_data);

    HttpResponse::build(actix_web::http::StatusCode::CREATED)
    .finish()
}

#[get("/metrics")]
async fn get_metrics(data: web::Data<MetricsData>) -> impl Responder {

    //convert data to string for printing with println
    let metrics_data = data.data.lock().unwrap();

    ENVIRONMENT_TEMPREATURE.with_label_values(&[&metrics_data.fds_dev_id, &metrics_data.fds_dev_loc]).set(metrics_data.temperature);
    ENVIRONMENT_HUMIDITY.with_label_values(&[&metrics_data.fds_dev_id, &metrics_data.fds_dev_loc]).set(metrics_data.humidity);
    HEAT_INDEX.with_label_values(&[&metrics_data.fds_dev_id, &metrics_data.fds_dev_loc]).set(metrics_data.heat_index);
    LITER_PER_HOUR.with_label_values(&[&metrics_data.fds_dev_id, &metrics_data.fds_dev_loc]).set(metrics_data.liter_per_hour);
    DISTANCE_IN_CM.with_label_values(&[&metrics_data.fds_dev_id, &metrics_data.fds_dev_loc]).set(metrics_data.distance_in_cm);
    WATER_LEVEL.with_label_values(&[&metrics_data.fds_dev_id, &metrics_data.fds_dev_loc]).set(metrics_data.water_level as i64);

    let encoder = TextEncoder::new();
    let metric_families = prometheus::gather();
    let mut buffer = vec![];

    encoder.encode(&metric_families, &mut buffer).unwrap();

    HttpResponse::Ok().content_type(encoder.format_type()).body(buffer)
}

#[get("/health")]
async fn health() -> impl Responder {
    HttpResponse::Ok().body("Healthy")
}