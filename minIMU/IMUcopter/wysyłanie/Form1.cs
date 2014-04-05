using System;
using System.Globalization;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using System.Timers;




namespace wysyłanie
{
    public partial class Form1 : Form
    {
        static AHRS.MadgwickAHRS AHRS = new AHRS.MadgwickAHRS(1f / 10048f, 2f);

        String ax, ay, az, mx, my, mz, gx, gy, gz, text;
        float fax, fay, faz, fmx, fmy, fmz, fgx, fgy, fgz;
        //double roll, aroll, caroll, pitch, apitch, capitch, yawn, ayawn, cayawn;
        float Yaw, Pitch, Roll;
        private static SerialPort port;

        private float[] eInt { get; set; }
        public float Ki { get; set; }
        public float Kp { get; set; }
        public float[] Quaternion { get; set; }
        public float SamplePeriod { get; set; }

        long Cstart;
        int loop=0;
        double point;

        public Form1()
        {
           
            SamplePeriod = 1f / 2097152f;
            Ki = 0f;
            Kp = 5f;
            Quaternion = new float[] { 1f, 0f, 0f, 0f };
            eInt = new float[] { 0f, 0f, 0f };
            
            InitializeComponent(); //inicjalizacja portu
            port = new SerialPort("COM5", 128000, Parity.None, 8, StopBits.One); //inicjalizacja portu

            port.ReadTimeout = 500; //ustawienie wartości timeout dla odczytu
            port.WriteTimeout = 500; //ustawienie wartości timeout dla wysyłania

            port.Handshake = Handshake.None; //ustawienie sterowania przepływu

            port.DataReceived += new SerialDataReceivedEventHandler(Port_DataReceived); //obsługa danych przychodzących
        }

        private void Port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            text = port.ReadLine();
            this.BeginInvoke(new EventHandler(drawCharts)); //wywołanie wątku wyświetlającego/przetwarzającego dane
            if (port.IsOpen)  //czyszczenie buforów
            {
                port.DiscardInBuffer();
                port.DiscardOutBuffer();
            }
        }

        private void drawCharts(object o, EventArgs e)
        {
            if (text.Length > 2)
            {
                String ktory = text.Substring(0, 2);

                switch (ktory)
                {
                    case "ax":
                        {
                            ax = text.Substring(2);
                            fax = float.Parse(ax);
                            this.richTextBox1.AppendText(ax);
                            this.richTextBox1.ScrollToCaret();
                            STM32_DataReceived_updateAHRS(fgx, fgy, fgz, fax, fay, faz, fmx, fmy, fmz);
                            break;
                        }
                    case "ay":
                        {
                            ay = text.Substring(2);
                            fay = float.Parse(ay);
                            this.richTextBox2.AppendText(ay);
                            this.richTextBox2.ScrollToCaret();
                            STM32_DataReceived_updateAHRS(fgx, fgy, fgz, fax, fay, faz, fmx, fmy, fmz);
                            break;
                        }
                    case "az":
                        {
                            az = text.Substring(2);
                            faz = float.Parse(az);
                            this.richTextBox3.AppendText(az);
                            this.richTextBox3.ScrollToCaret();
                            STM32_DataReceived_updateAHRS(fgx, fgy, fgz, fax, fay, faz, fmx, fmy, fmz);
                            break;
                        }
                    case "mx":
                        {
                            mx = text.Substring(2);
                            fmx = float.Parse(mx);
                            this.richTextBox4.AppendText(mx);
                            this.richTextBox4.ScrollToCaret();
                            STM32_DataReceived_updateAHRS(fgx, fgy, fgz, fax, fay, faz, fmx, fmy, fmz);
                            break;
                        }
                    case "my":
                        {
                            my = text.Substring(2);
                            fmy = float.Parse(my);
                            this.richTextBox5.AppendText(my);
                            this.richTextBox5.ScrollToCaret();
                            STM32_DataReceived_updateAHRS(fgx, fgy, fgz, fax, fay, faz, fmx, fmy, fmz);
                            break;
                        }
                    case "mz":
                        {
                            mz = text.Substring(2);
                            fmz = float.Parse(mz);
                            this.richTextBox6.AppendText(mz);
                            this.richTextBox6.ScrollToCaret();
                            STM32_DataReceived_updateAHRS(fgx, fgy, fgz, fax, fay, faz, fmx, fmy, fmz);
                            break;
                        }
                    case "gx":
                        {
                            gx = text.Substring(2);
                            fgx = float.Parse(gx);
                            this.richTextBox9.AppendText(gx);
                            this.richTextBox9.ScrollToCaret();
                            STM32_DataReceived_updateAHRS(fgx, fgy, fgz, fax, fay, faz, fmx, fmy, fmz);
                            break;
                        }
                    case "gy":
                        {
                            gy = text.Substring(2);
                            fgy = float.Parse(gy);
                            this.richTextBox8.AppendText(gy);
                            this.richTextBox8.ScrollToCaret();
                            STM32_DataReceived_updateAHRS(fgx, fgy, fgz, fax, fay, faz, fmx, fmy, fmz);
                            break;
                        }
                    case "gz":
                        {
                            gz = text.Substring(2);
                            fgz = float.Parse(gz);
                            this.richTextBox7.AppendText(gz);
                            this.richTextBox7.ScrollToCaret();
                            STM32_DataReceived_updateAHRS(fgx, fgy, fgz, fax, fay, faz, fmx, fmy, fmz);
                            
                            loop++;
                            //point = loop;
                             point = ((DateTime.Now.Ticks - Cstart) / 10000000);
                             if (point > 0) point = loop / point;
                            this.textBox4.Text = point.ToString("0,0.00", CultureInfo.InvariantCulture); ;
                           // MAGR_DataReceived_3D(ref Yaw, ref Pitch, ref Roll);
                            break;
                        }
                }
                //STM32_DataReceived_updateAHRS(fgx, fgy, fgz, fax, fay, faz, fmx, fmy, fmz);
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            port.Open();
            if (!port.IsOpen)
            {
                MessageBox.Show("Error", "Cannot open COM port", MessageBoxButtons.OK, MessageBoxIcon.Error);
                this.stanstm.Text = "Offline";
                this.stanstm.ForeColor = System.Drawing.Color.Red;
            }
            else
            {
                this.stanstm.Text = "Online";
                this.stanstm.ForeColor = System.Drawing.Color.Green;
                this.start.Enabled = false;
                this.stop.Enabled = true;
                Cstart = DateTime.Now.Ticks;    
            }
        }






       
        public void AHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz)
        {
            float q1 = Quaternion[0], q2 = Quaternion[1], q3 = Quaternion[2], q4 = Quaternion[3];   // short name local variable for readability
            float norm;
            float hx, hy, bx, bz;
            float vx, vy, vz, wx, wy, wz;
            float ex, ey, ez;
            float pa, pb, pc;

            // Auxiliary variables to avoid repeated arithmetic
            float q1q1 = q1 * q1;
            float q1q2 = q1 * q2;
            float q1q3 = q1 * q3;
            float q1q4 = q1 * q4;
            float q2q2 = q2 * q2;
            float q2q3 = q2 * q3;
            float q2q4 = q2 * q4;
            float q3q3 = q3 * q3;
            float q3q4 = q3 * q4;
            float q4q4 = q4 * q4;

            // Normalise accelerometer measurement
            norm = (float)Math.Sqrt(ax * ax + ay * ay + az * az);
            if (norm == 0f) return; // handle NaN
            norm = 1 / norm;        // use reciprocal for division
            ax *= norm;
            ay *= norm;
            az *= norm;

            // Normalise magnetometer measurement
            norm = (float)Math.Sqrt(mx * mx + my * my + mz * mz);
            if (norm == 0f) return; // handle NaN
            norm = 1 / norm;        // use reciprocal for division
            mx *= norm;
            my *= norm;
            mz *= norm;

            // Reference direction of Earth's magnetic field
            hx = 2f * mx * (0.5f - q3q3 - q4q4) + 2f * my * (q2q3 - q1q4) + 2f * mz * (q2q4 + q1q3);
            hy = 2f * mx * (q2q3 + q1q4) + 2f * my * (0.5f - q2q2 - q4q4) + 2f * mz * (q3q4 - q1q2);
            bx = (float)Math.Sqrt((hx * hx) + (hy * hy));
            bz = 2f * mx * (q2q4 - q1q3) + 2f * my * (q3q4 + q1q2) + 2f * mz * (0.5f - q2q2 - q3q3);

            // Estimated direction of gravity and magnetic field
            vx = 2f * (q2q4 - q1q3);
            vy = 2f * (q1q2 + q3q4);
            vz = q1q1 - q2q2 - q3q3 + q4q4;
            wx = 2f * bx * (0.5f - q3q3 - q4q4) + 2f * bz * (q2q4 - q1q3);
            wy = 2f * bx * (q2q3 - q1q4) + 2f * bz * (q1q2 + q3q4);
            wz = 2f * bx * (q1q3 + q2q4) + 2f * bz * (0.5f - q2q2 - q3q3);

            // Error is cross product between estimated direction and measured direction of gravity
            ex = (ay * vz - az * vy) + (my * wz - mz * wy);
            ey = (az * vx - ax * vz) + (mz * wx - mx * wz);
            ez = (ax * vy - ay * vx) + (mx * wy - my * wx);
            if (Ki > 0f)
            {
                eInt[0] += ex;      // accumulate integral error
                eInt[1] += ey;
                eInt[2] += ez;
            }
            else
            {
                eInt[0] = 0.0f;     // prevent integral wind up
                eInt[1] = 0.0f;
                eInt[2] = 0.0f;
            }

            // Apply feedback terms
            gx = gx + Kp * ex + Ki * eInt[0];
            gy = gy + Kp * ey + Ki * eInt[1];
            gz = gz + Kp * ez + Ki * eInt[2];

            // Integrate rate of change of quaternion
            pa = q2;
            pb = q3;
            pc = q4;
            q1 = q1 + (-q2 * gx - q3 * gy - q4 * gz) * (0.5f * SamplePeriod);
            q2 = pa + (q1 * gx + pb * gz - pc * gy) * (0.5f * SamplePeriod);
            q3 = pb + (q1 * gy - pa * gz + pc * gx) * (0.5f * SamplePeriod);
            q4 = pc + (q1 * gz + pa * gy - pb * gx) * (0.5f * SamplePeriod);

            // Normalise quaternion
            norm = (float)Math.Sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);
            norm = 1.0f / norm;
            Quaternion[0] = q1 * norm;
            Quaternion[1] = q2 * norm;
            Quaternion[2] = q3 * norm;
            Quaternion[3] = q4 * norm;

            float[] quat = new float[4];
            quat[0] = Quaternion[0];
            quat[1] = Quaternion[1];
            quat[2] = Quaternion[2];
            quat[3] = Quaternion[3];

            //  float q02 = quat[0] * quat[0];
            float q12 = quat[1] * quat[1];
            float q22 = quat[2] * quat[2];
            float q32 = quat[3] * quat[3];

            Roll = rad2deg((float)Math.Atan2(2 * (quat[2] * quat[3] + quat[0] * quat[1]), (1 - 2 * (q12 + q22))));
            Pitch = rad2deg((float)-Math.Asin(2 * (quat[1] * quat[3] - quat[0] * quat[2])));
            Yaw = rad2deg((float)Math.Atan2(2 * (quat[1] * quat[2] + quat[0] * quat[3]), (1 - 2 * (q22 + q32))));

            this.textBox1.Text = Roll.ToString("0,0.00", CultureInfo.InvariantCulture); ;
            this.textBox2.Text = Pitch.ToString("0,0.00", CultureInfo.InvariantCulture); ;
            this.textBox3.Text = Yaw.ToString("0,0.00", CultureInfo.InvariantCulture); ;


            this.chart1.Series["Pitch"].Points.AddY(Pitch);
            this.chart1.Series["Roll"].Points.AddY(Roll);
            this.chart1.Series["Yawn"].Points.AddY(Yaw);
        }
     

        private void button5_Click_1(object sender, EventArgs e)
        {
            if (port.IsOpen)
            {
                port.Close();
                this.start.Enabled = true;
                this.stop.Enabled = false;
            }
        }


        private void MAGR_DataReceived_3D(ref float Yaw, ref float Pitch, ref float Roll)
        {
            float[] quat = new float[4];
            quat[0] = AHRS.Quaternion[0];
            quat[1] = AHRS.Quaternion[1];
            quat[2] = AHRS.Quaternion[2];
            quat[3] = AHRS.Quaternion[3];

            //  float q02 = quat[0] * quat[0];
            float q12 = quat[1] * quat[1];
            float q22 = quat[2] * quat[2];
            float q32 = quat[3] * quat[3];

            Roll = rad2deg((float)Math.Atan2(2 * (quat[2] * quat[3] + quat[0] * quat[1]), (1 - 2 * (q12 + q22))));
            Pitch = rad2deg((float)-Math.Asin(2 * (quat[1] * quat[3] - quat[0] * quat[2])));
            Yaw = rad2deg((float)Math.Atan2(2 * (quat[1] * quat[2] + quat[0] * quat[3]), (1 - 2 * (q22 + q32))));

            this.textBox1.Text = Roll.ToString("0,0.00", CultureInfo.InvariantCulture); ;
            this.textBox2.Text = Pitch.ToString("0,0.00", CultureInfo.InvariantCulture); ;
            this.textBox3.Text = Yaw.ToString("0,0.00", CultureInfo.InvariantCulture); ;


            this.chart1.Series["Pitch"].Points.AddY(Pitch);
            this.chart1.Series["Roll"].Points.AddY(Roll);
            this.chart1.Series["Yawn"].Points.AddY(Yaw);
        }

        public void STM32_DataReceived_updateAHRS(float _gx, float _gy, float _gz, float _ax, float _ay, float _az, float _mx, float _my, float _mz)
        {
           // AHRS.Update(deg2rad(_gx), deg2rad(_gy), deg2rad(_gz), _ax, _ay, _az, _mx, _my, _mz);
            //AHRSupdate(deg2rad(_gx), deg2rad(_gy), deg2rad(_gz), _ax, _ay, _az, _mx, _my, _mz);
            AHRSupdate((_gx), (_gy), (_gz), _ax, _ay, _az, _mx, _my, _mz);
        }

        static float deg2rad(float degrees)
        {
            return (float)(Math.PI / 180) * degrees;
        }

        static float rad2deg(float radians)
        {
            return (radians * 180) / (float)Math.PI;
        }

    }
}