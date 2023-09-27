using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using Notlnfekt;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;
using System.Text;
using System.Threading;

namespace Notlnfekt
{
    public partial class Page2 : Page
    {
        private Process myProcess;
        private const string RootDir = "C:\\";
        private const string ServiceName = "NotInfecktya";
        public MainWindow MainWindow { get; set; }

        public Page2()
        {
            InitializeComponent();
        }

        Color customColor = Color.FromRgb(0xFA, 0x1E, 0x4E);
        private async void btn_soldiers_Click(object sender, RoutedEventArgs e)
        {
            var fadeAnimation = new DoubleAnimation();
            fadeAnimation.From = 1;
            fadeAnimation.To = 0;
            fadeAnimation.AutoReverse = true;
            lbl_titulo.BeginAnimation(Label.OpacityProperty, fadeAnimation);

            btn_soldiers.IsEnabled = false;
            progressBar.Visibility = Visibility.Visible;
            loadingMessage.Visibility = Visibility.Visible;

            await Task.Run(() => DescobreDiretorios(RootDir));
            MostraMessageBox("Soldados posicionados");

            await Task.Delay(1000);

            
            var colorAnimation = new ColorAnimation();
            colorAnimation.To = Colors.Green;
            colorAnimation.Duration = new Duration(TimeSpan.FromSeconds(1)); 

            
            var storyboard = new Storyboard();
            storyboard.Children.Add(colorAnimation);

            
            Storyboard.SetTarget(colorAnimation, progressBar);
            Storyboard.SetTargetProperty(colorAnimation, new PropertyPath("Background.Color"));

            
            storyboard.Begin();

           
            loadingMessage.Text = "🥳🎉🥳🎉";
            progressBar.Value = 100; 
            loadingMessage.Visibility = Visibility.Hidden; 

            btn_soldiers.IsEnabled = true;
        }



        private async Task DescobreDiretorios(string dirpath)
        {
            try
            {
                var folders = Directory.GetDirectories(dirpath).Where(dir => !dir.EndsWith(".") && !dir.EndsWith("..")).ToList();
                var totalFolders = folders.Count;
                var completedFolders = 0;

                foreach (var folder in folders)
                {
                    ListFoldersInDirectory(folder);
                    await CriaSoldados(folder);

                    completedFolders++;
                    double progress = (double)completedFolders / totalFolders * 100;
                    barra_de_carregamento(progress);

                   
                    await DescobreDiretorios(folder);
                }
            }
            catch (UnauthorizedAccessException e)
            {
                Console.WriteLine("Erro de acesso: " + e.Message);
            }
            catch (DirectoryNotFoundException e)
            {
                Console.WriteLine("Diretório não encontrado: " + e.Message);
            }
            catch (Exception e)
            {
                Console.WriteLine("Erro desconhecido ao percorrer pastas em " + dirpath + ": " + e.Message);
            }
        }

        private async Task CriaSoldados(string dirname)
        {
            string[] filenames = { ".__Infecktya", "aaInfecktya", "zzInfecktya", ".__Infecktya.txt", "aaInfecktya.txt", "zzInfecktya.txt", ".__Infecktya.docx", "aaInfecktya.docx", "zzInfecktya.docx" };
            int totalFiles = filenames.Length;
            int completedFiles = 0;
            bool mensagemExibidaLocal = false;

            try
            {
                if (Directory.Exists(dirname))
                {
                    foreach (string filename in filenames)
                    {
                        string path = Path.Combine(dirname, filename);

                        await File.WriteAllTextAsync(path, string.Empty);
                        File.SetAttributes(path, File.GetAttributes(path) | FileAttributes.Hidden);

                        completedFiles++;

                        double progress = (double)completedFiles / totalFiles * 100;
                        barra_de_carregamento(progress);

                        if (!mensagemExibidaLocal && File.Exists(path))
                        {
                            mensagemExibidaLocal = true;
                        }
                    }
                    
                }
                else
                {
                    Console.WriteLine($"O diretório {dirname} não existe.");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Erro ao acessar o diretório {dirname}: {ex.Message}");
            }
        }


        private void ListFoldersInDirectory(string dirpath)
        {
            try
            {
                var folders = Directory.GetDirectories(dirpath).Where(dir => !dir.EndsWith(".") && !dir.EndsWith("..")).ToList();
                foreach (string folder in folders)
                {
                    Console.WriteLine(folder);
                }
            }
            catch (UnauthorizedAccessException e)
            {
                Console.WriteLine("Erro de acesso: " + e.Message);
            }
            catch (DirectoryNotFoundException e)
            {
                Console.WriteLine("Diretório não encontrado: " + e.Message);
            }
            catch (Exception e)
            {
                Console.WriteLine("Erro desconhecido ao listar pastas em " + dirpath + ": " + e.Message);
            }
        }

       
        private void barra_de_carregamento(double value)
        {
            Dispatcher.Invoke(() => progressBar.Value = value);
        }


        private void MostraMessageBox(string message)
        {
            Dispatcher.Invoke(() => MessageBox.Show(message));
        }

        private void ExecuteServiceCommand(string command, Button button)
        {
            string cmd = $"/C sc {command} {ServiceName}";
            ProcessStartInfo psi = new ProcessStartInfo("cmd.exe", cmd);
            psi.CreateNoWindow = true;
            psi.UseShellExecute = false;
            psi.RedirectStandardOutput = true;
            psi.Verb = "runas";
            Process process = new Process();
            process.StartInfo = psi;
            process.Start();
            process.WaitForExit();
            string output = process.StandardOutput.ReadToEnd();
            int exitCode = process.ExitCode;
            if (exitCode == 0)
            {
                button.Content = "🥳🎉🥳🎉";
                button.Background = Brushes.Green;
                var timer = new System.Windows.Threading.DispatcherTimer();
                timer.Tick += (s, args) =>
                {
                    change();
                    button.Background = Brushes.Black;
                    timer.Stop();
                };
                timer.Interval = TimeSpan.FromSeconds(4);
                timer.Start();
            }
            else
            {
                button.Content = "😱😱😱";
                SolidColorBrush customBrush = new SolidColorBrush(customColor);
                button.Background = customBrush;
                var timer = new System.Windows.Threading.DispatcherTimer();
                timer.Tick += (s, args) =>
                {
                    change();
                    button.Background = Brushes.Black;
                    timer.Stop();
                };
                timer.Interval = TimeSpan.FromSeconds(4);
                timer.Start();
            }
        }

        // Functions
        [DllImport("fltlib.dll", SetLastError = true)]
        public static extern uint FilterConnectCommunicationPort(
            [MarshalAs(UnmanagedType.LPWStr)] string PortName,
            uint Options,
            IntPtr Context,
            uint SizeOfContext,
            IntPtr SecurityAttributes,
            out SafeFileHandle PortHandle
        );
        [DllImport("fltlib.dll", SetLastError = true)]
        public static extern int FilterSendMessage(
             SafeFileHandle hPort,
             IntPtr lpInBuffer,
             uint dwInBufferSize,
             IntPtr lpOutBuffer,
             uint dwOutBufferSize,
             out uint lpBytesReturned
         );

        

        //liga o driver
        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            ExecuteServiceCommand("start", btn_liga);

            
            DriverComunication.result = FilterConnectCommunicationPort("\\NotInfecktya", 0, IntPtr.Zero, 0, IntPtr.Zero, out DriverComunication.portHandle);

            if (DriverComunication.result == 0)
            {
                MostraMessageBox("Connected to minifilter communication port successfully!");

                int intValueToSend = 0;
                IntPtr lpInBuffer = Marshal.AllocHGlobal(sizeof(int));
                Marshal.WriteInt32(lpInBuffer, intValueToSend);
                uint dwInBufferSize = (uint)sizeof(int);

                uint dwOutBufferSize = 1024;
                IntPtr lpOutBuffer = Marshal.AllocHGlobal((int)dwOutBufferSize);
                uint lpBytesReturned;

                int sendMessageResult = FilterSendMessage(DriverComunication.portHandle, lpInBuffer, dwInBufferSize, lpOutBuffer, dwOutBufferSize, out lpBytesReturned);

                if (sendMessageResult == 0)
                {
                    Console.WriteLine("FilterSendMessage succeeded.");

                    int response = Marshal.ReadInt32(lpOutBuffer);
                    tb_token.Text = response.ToString();
                    
                    Console.WriteLine("Response from MiniFilter: " + response);
                }
                else
                {
                    Console.WriteLine("FilterSendMessage failed with HRESULT 0x" + sendMessageResult.ToString("X"));
                }

                // Clean up allocated memory
                Marshal.FreeHGlobal(lpInBuffer);
                Marshal.FreeHGlobal(lpOutBuffer);
                DriverComunication.portHandle.Close();

               
            }
            else
            {
                MostraMessageBox("Failed to connect to minifilter communication port. Error code: " + DriverComunication.result);
            }
        }

        //desliga o driver

        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            DriverComunication.portHandle.Close();
            ExecuteServiceCommand("stop", btn_off);   
        }

        //muda linguagem
        void change()
        {
            if (model_configs.lang == "en")
            {
                btn_liga.Content = "On";
                btn_off.Content = "Off";
                btn_soldiers.Content = "Add Soldiers";
                loadingMessage.Text = "Loading...";
                btn_recupera_Copiar.Content = "Recover";
            }
            else if (model_configs.lang == "pt")
            {
                btn_liga.Content = "Ligar";
                btn_off.Content = "Desligar";
                btn_soldiers.Content = "Add Soldados";
                loadingMessage.Text = "Carregando...";
                btn_recupera_Copiar.Content = "Recuperar";
            }
            else if (model_configs.lang == "es")
            {
                btn_liga.Content = "Encender";
                btn_off.Content = "Apagar";
                btn_soldiers.Content = "Add Soldados";
                loadingMessage.Text = "Cargando...";
                btn_recupera_Copiar.Content = "Recuperar";
            }
        }

        private void Page_Loaded(object sender, RoutedEventArgs e)
        {
            change();
        }

        //abre a pasta do backup
        private void btn_recupera_Copiar_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                string pastaPath = @"C:\BackFromDead";

                ProcessStartInfo startInfo = new ProcessStartInfo
                {
                    FileName = "explorer.exe",
                    Arguments = pastaPath,
                    WorkingDirectory = pastaPath
                };

                Process.Start(startInfo);
            }
            catch 
            {
                MostraMessageBox("Desligue o driver para poder abrir o backup");
            }
        }

        private void btn_apaga_Copiar_Click(object sender, RoutedEventArgs e)
        {
            System.IO.DirectoryInfo di = new DirectoryInfo(@"C:\BackFromDead\");

            foreach (FileInfo file in di.GetFiles())
            {
                file.Delete();
            }          
        }

    }

}
