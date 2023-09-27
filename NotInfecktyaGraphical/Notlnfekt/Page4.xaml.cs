using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Notlnfekt
{
    
    public partial class Page4 : Page
    {
        public Page4()
        {
            InitializeComponent();
        }

        private void Page_Loaded(object sender, RoutedEventArgs e)
        {
            change();
        }

        //muda linguagem 
        void change()
        {
            if (model_configs.lang == "en")
            {
                text_dim.Text = "Dear user,\r\nI would like to introduce our Startup and an incredible opportunity for you to support our mission.\r\nAt root@Acch, we have developed an innovative kernel driver that can detect ransomware attacks in real-time, providing a robust defense against this growing threat.\r\nHowever, we need your help to bring this groundbreaking technology to the forefront. Your contribution will directly support our research, development, and testing efforts, enabling us to make a lasting impact in the fight against ransomware.\r\nPlease send our donation on the QR code to contribute and support our mission. Every donation, no matter the size, is greatly appreciated and will help us secure the digital world we rely on.\r\nThank you for considering supporting our startup. Together, we can make a significant difference in combating ransomware and protecting businesses and individuals from its devastating effects.\r\nWarm regards.";
                text_dim.Foreground = new SolidColorBrush(Color.FromRgb(255, 255, 255));
            }
            else if (model_configs.lang == "pt")
            {
                text_dim.Text = "Caro usuário,\r\nGostaria de apresentar nossa Startup e uma oportunidade incrível para você apoiar nossa missão.\r\nNo root@Acch, desenvolvemos um driver de kernel inovador que pode detectar ataques de ransomware em tempo real, fornecendo uma defesa robusta contra essa ameaça crescente.\r\nNo entanto, precisamos da sua ajuda para trazer essa tecnologia inovadora para a vanguarda. Sua contribuição apoiará diretamente nossos esforços de pesquisa, desenvolvimento e testes, permitindo-nos fazer um impacto duradouro na luta contra o ransomware.\r\nEnvie nossa doação no código QR para contribuir e apoiar nossa missão. Cada doação, não importa o tamanho, é muito apreciada e nos ajudará a proteger o mundo digital em que confiamos.\r\nObrigado por considerar apoiar nossa startup. Juntos, podemos fazer uma diferença significativa no combate ao ransomware e na proteção de empresas e indivíduos de seus efeitos devastadores.\r\nAtenciosamente.";
                text_dim.Foreground = new SolidColorBrush(Color.FromRgb(255, 255, 255));
            }
            else if (model_configs.lang == "es")
            {
                text_dim.Text = "Estimado usuario,\r\nMe gustaría presentar nuestra Startup y una increíble oportunidad para que apoyes nuestra misión.\r\nEn root@Acch, hemos desarrollado un controlador de kernel innovador que puede detectar ataques de ransomware en tiempo real, proporcionando una defensa sólida contra esta creciente amenaza.\r\nSin embargo, necesitamos tu ayuda para llevar esta tecnología revolucionaria a la vanguardia. Tu contribución apoyará directamente nuestros esfuerzos de investigación, desarrollo y pruebas, lo que nos permitirá tener un impacto duradero en la lucha contra el ransomware.\r\nPor favor, envía tu donación en el código QR para contribuir y respaldar nuestra misión. Cada donación, sin importar el monto, es muy apreciada y nos ayudará a proteger el mundo digital en el que confiamos.\r\nGracias por considerar apoyar nuestra startup. Juntos, podemos hacer una diferencia significativa en la lucha contra el ransomware y en la protección de empresas e individuos de sus efectos devastadores.\r\nSaludos cordiales.";
                text_dim.Foreground = new SolidColorBrush(Color.FromRgb(255, 255, 255));
            }
        }
        //copia carteira
        private void CopyToClipboard_Click(object sender, RoutedEventArgs e)
        {
            
            string textoParaCopiar = "0xc66982F418031D1CB9cfCFa9Ae4384DD826DA0CC";

            
            Clipboard.SetText(textoParaCopiar);

            
            var botao = (Button)sender;
            botao.Content = "🥳🎉🥳🎉";
            botao.Background = Brushes.Green;
            
           
            var timer = new System.Windows.Threading.DispatcherTimer();
            timer.Tick += (s, args) =>
            {
                botao.Content = "Copiar";
                botao.Background = Brushes.Black;
                timer.Stop();
            };
            timer.Interval = TimeSpan.FromSeconds(2); 
            timer.Start();
        }
    }
}
