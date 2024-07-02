<?php

include 'connection.php';

if (isset($_POST['uid']) && isset($_POST['option'])) {
    $id = $_POST['uid'];
    $option = $_POST['option'];
    
    // Cek opsi yang dipilih
    switch ($option) {
        case 'isi_saldo':
            if (isset($_POST['amount'])) {
                $amount = (int)$_POST['amount'];
                $query = mysqli_query($dbconnect, "SELECT * FROM tb_user WHERE id='$id'");
                $cek = mysqli_num_rows($query);
                if ($cek > 0) {
                    $data = mysqli_fetch_assoc($query);
                    $hasil_saldo = $data['saldo'] + $amount;
                    mysqli_query($dbconnect, "UPDATE tb_user SET saldo ='$hasil_saldo' WHERE id='$id'");
                    $status = "Saldo berhasil diisi";
                } else {
                    $status = "ID Belum Terdaftar";
                    $data = "-";
                    $amount = "-";
                    $hasil_saldo = "-";
                }
            } else {
                $status = "Jumlah saldo tidak valid";
                $data = "-";
                $amount = "-";
                $hasil_saldo = "-";
            }
            break;
            
        case 'pembayaran':
            if (isset($_POST['amount'])) {
                $amount = (int)$_POST['amount'];
                $query = mysqli_query($dbconnect, "SELECT * FROM tb_user WHERE id='$id'");
                $cek = mysqli_num_rows($query);
                if ($cek > 0) {
                    $data = mysqli_fetch_assoc($query);
                    $hasil_saldo = $data['saldo'] - $amount;
                    if ($hasil_saldo >= 0) {
                        mysqli_query($dbconnect, "UPDATE tb_user SET saldo ='$hasil_saldo' WHERE id='$id'");
                        $status = "Transaksi Sukses";
                    } else {
                        $status = "Saldo tidak mencukupi";
                        $data = "-";
                        $amount = "-";
                        $hasil_saldo = "-";
                    }
                } else {
                    $status = "ID Belum Terdaftar";
                    $data = "-";
                    $amount = "-";
                    $hasil_saldo = "-";
                }
            } else {
                $status = "Jumlah pembayaran tidak valid";
                $data = "-";
                $amount = "-";
                $hasil_saldo = "-";
            }
            break;
            
        case 'cek_saldo':
            $query = mysqli_query($dbconnect, "SELECT * FROM tb_user WHERE id='$id'");
            $cek = mysqli_num_rows($query);
            if ($cek > 0) {
                $data = mysqli_fetch_assoc($query);
                $status = "Cek Saldo";
                $amount = "-";
                $hasil_saldo = $data['saldo'];
            } else {
                $status = "ID Belum Terdaftar";
                $data = "-";
                $amount = "-";
                $hasil_saldo = "-";
            }
            break;
            
        case 'daftar_rfid':
            // Memasukkan data RFID baru ke dalam database
            if (isset($_POST['nama'])) {
                $nama = $_POST['nama'];
                $query = mysqli_query($dbconnect, "INSERT INTO tb_user (id, nama, saldo) VALUES ('$id', '$nama', 0)");
                if ($query) {
                    $status = "RFID berhasil didaftarkan";
                    $data = "-";
                    $amount = "-";
                    $hasil_saldo = "-";
                } else {
                    $status = "Gagal mendaftarkan RFID";
                    $data = "-";
                    $amount = "-";
                    $hasil_saldo = "-";
                }
            } else {
                $status = "Nama tidak tersedia";
                $data = "-";
                $amount = "-";
                $hasil_saldo = "-";
            }
            break;

        default:
            $status = "Opsi tidak valid";
            $data = "-";
            $amount = "-";
            $hasil_saldo = "-";
            break;
    }
    
    // Format output sebagai JSON
    $output = [
        "Detail" => [
            "Status" => $status,
            "Data User" => $data,
            "Nilai Transaksi" => $amount,
            "Saldo Akhir" => (int)$hasil_saldo
        ]
    ];
    
    $json = json_encode($output);
    echo $json;
} else {
    echo "Invalid Request";
}

?>
