<?php

// --- 1. Konfigurasi Database ---
// Ganti dengan kredensial database Anda
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "hidroponik_db";

// Buat koneksi
$conn = new mysqli($servername, $username, $password, $dbname);

// Cek koneksi
if ($conn->connect_error) {
    die("Koneksi gagal: " . $conn->connect_error);
}

// Inisialisasi array respons utama
$response = [];

// --- 2. Query Statistik Dasar (Max, Min, Avg) ---
$sql_stats = "SELECT MAX(suhu) AS suhumax, MIN(suhu) AS suhumin, AVG(suhu) AS suhurata FROM data_sensor";
$result_stats = $conn->query($sql_stats);
$stats = $result_stats->fetch_assoc();

// Masukkan statistik ke respons (pastikan di-casting ke angka)
$response['suhumax'] = (float) $stats['suhumax'];
$response['suhumin'] = (float) $stats['suhumin'];
$response['suhurata'] = (float) $stats['suhurata'];

// Simpan nilai suhumax untuk query berikutnya
$suhu_max_value = $stats['suhumax'];


// --- 3. Query untuk Data Detail pada Suhu Maksimal ---
// Kita ambil semua baris data yang suhunya sama dengan suhumax
// Kita juga format 'timestamp' menjadi 'YYYY-MM' untuk pengelompokan
$sql_details = "SELECT 
                    id, 
                    suhu, 
                    humidity AS humid, 
                    lux AS kecerahan, 
                    timestamp,
                    DATE_FORMAT(timestamp, '%Y-%m') AS month_year
                FROM data_sensor 
                WHERE suhu = ?";

// Gunakan prepared statement untuk keamanan
$stmt = $conn->prepare($sql_details);
$stmt->bind_param("d", $suhu_max_value); // 'd' untuk double
$stmt->execute();
$result_details = $stmt->get_result();

// --- 4. Proses Hasil Query Detail ---
$nilai_suhu_max_array = [];
$month_year_temp_list = []; // Untuk menampung 'month_year' sementara

if ($result_details->num_rows > 0) {
    while($row = $result_details->fetch_assoc()) {
        
        // Kumpulkan data untuk array 'nilai_suhu_max_humid_max'
        $nilai_suhu_max_array[] = [
            'idx' => (int) $row['id'],
            'suhun' => (float) $row['suhu'], // 'suhun' di gambar, mungkin typo dari 'suhu'
            'humid' => (float) $row['humid'],
            'kecerahan' => (float) $row['kecerahan'],
            'timestamp' => $row['timestamp']
        ];

        // Kumpulkan data untuk array 'month_year_max'
        $month_year_temp_list[] = $row['month_year'];
    }
}

// Masukkan array data detail ke respons
$response['nilai_suhu_max_humid_max'] = $nilai_suhu_max_array;


// --- 5. Proses 'month_year_max' ---
// Buat daftar unik dari 'month_year' yang didapat
$unique_month_years = array_unique($month_year_temp_list);

$month_year_max_array = [];
foreach ($unique_month_years as $my) {
    // Format ke dalam bentuk array of objects
    $month_year_max_array[] = ['month_year' => $my];
}

// Masukkan array month_year ke respons
$response['month_year_max'] = $month_year_max_array;


// --- 6. Kirim Respons sebagai JSON ---
header('Content-Type: application/json');
echo json_encode($response, JSON_PRETTY_PRINT); // JSON_PRETTY_PRINT agar rapi

// Tutup koneksi
$stmt->close();
$conn->close();

?>