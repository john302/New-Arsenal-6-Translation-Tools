using System;
using System.IO;
using System.Text;
using System.Xml;
using YandexTranslateCSharpSdk;

namespace XMLTranslator
{
    class Program
    {
        static YandexTranslateSdk wrapper = new YandexTranslateSdk();

        static void Main(string[] args)
        {
            string encoding = ReadSettings("encoding");
            string apikey = ReadSettings("apikey");

            if (args.Length < 1)
            {
                Console.WriteLine("Invalid command line argument.");
                Console.WriteLine("Pass a directory of XML files as an argument to the program, or drag and drop a folder onto the .exe");
                Console.ReadKey();
                return;
            }

            //used for yandex translate api
            wrapper.ApiKey = apikey;

            StreamWriter logFile = new StreamWriter(".\\outputXML.log", true);

            Console.WriteLine("Conversion Started on {0}", DateTime.Now.ToString());
            logFile.WriteLine("----- Conversion Started on {0}", DateTime.Now.ToString());
            logFile.Close();
            
            //create the output directory
            Directory.CreateDirectory(".\\outputXML");

            //get all xml files in the drag and dropped folder
            string[] xmlFilesSource = Directory.GetFiles(args[0], "*.xml", SearchOption.TopDirectoryOnly);

            for (int ii = 0; ii < xmlFilesSource.Length; ii++)
            {
                logFile = new StreamWriter(".\\outputXML.log", true);

                Console.WriteLine("Processing File: {0}", xmlFilesSource[ii]);
                logFile.WriteLine("--- Processing File: {0}", xmlFilesSource[ii]);

                //read the file contents into a string buffer, for writing out later
                StreamReader fileReader = new StreamReader(xmlFilesSource[ii], Encoding.GetEncoding(encoding));
                string fileContents = fileReader.ReadToEnd();
                fileReader.Close();

                //some of the xml files have illegal characters and bad formatting that break the xml parser, this fixes known issues
                fileContents = FirstPass(fileContents);
                File.WriteAllText(xmlFilesSource[ii], fileContents, Encoding.GetEncoding(encoding));

                //open the file for xml parsing
                fileReader = new StreamReader(xmlFilesSource[ii], Encoding.GetEncoding(encoding));
                XmlReaderSettings xmlSettings = new XmlReaderSettings();
                xmlSettings.ConformanceLevel = ConformanceLevel.Fragment;
                XmlReader xmlFile = XmlReader.Create(fileReader, xmlSettings);

                //iterate through all xml nodes
                while (xmlFile.Read())
                {
                    //once we reach a text element, translate it
                    if ((xmlFile.NodeType == XmlNodeType.Element) && (xmlFile.LocalName == "text"))
                    {
                        try
                        {
                            //the FormatString and UnformatString functions preserve C-style escape chars through translation
                            string russian = xmlFile.ReadInnerXml();
                            string english = FormatString(TranslateRussian(UnformatString(russian)));

                            //replace the russian string with the english one in our fileContents buffer
                            fileContents = ReplaceFirst(fileContents, russian, english);
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("Exception encountered, check the log file.");
                            Console.WriteLine("Translation stopped, press any key to exit.");
                            logFile.WriteLine("ERROR:");
                            logFile.WriteLine("- ! The application encountered an exception.");
                            logFile.WriteLine("- ! This is probably due to Yandex failing to translate something. (Over quota/Invalid input string)");
                            logFile.WriteLine("EXCEPTION:");
                            logFile.WriteLine(ex.ToString());
                            logFile.Close();
                            Console.ReadKey();
                            return;
                        }
                    }
                }

                fileReader.Close();     //we're done with this XML file

                //write the file to the output directory
                string filePath = ".\\outputXML\\" + Path.GetFileName(xmlFilesSource[ii]);
                File.WriteAllText(filePath, fileContents, Encoding.GetEncoding(encoding));

                Console.WriteLine("Completed File: {0}", filePath);
                logFile.WriteLine("--- Completed File: {0}\n", filePath);
                
                logFile.Close();
            }

            logFile = new StreamWriter(".\\outputXML.log", true);
            logFile.WriteLine("----- Conversion Completed {0}", DateTime.Now);
            logFile.WriteLine("\n\n\n");
            logFile.Close();
            Console.WriteLine("Conversion Completed {0}", DateTime.Now);

            Console.WriteLine("Some files may need to be manually translated and/or edited.");
            Console.WriteLine("Press any key to exit.");
            Console.ReadKey();
            return;
        }

        static string ReadSettings(string field)
        {
            if (File.Exists(".\\settings.xml"))
            {
                XmlReaderSettings xmlSettings = new XmlReaderSettings();
                xmlSettings.ConformanceLevel = ConformanceLevel.Fragment;
                XmlReader xmlFile = XmlReader.Create(".\\settings.xml", xmlSettings);

                while (xmlFile.Read())
                {
                    if (xmlFile.NodeType == XmlNodeType.Element && xmlFile.Name == field)
                    {
                        return xmlFile.ReadInnerXml();
                    }
                }
                xmlFile.Close();
            }
            else
            {
                string def = "<apikey>yandex api key goes here</apikey>\n<encoding>windows-1251</encoding>";
                File.WriteAllText(".\\settings.xml", def);
                Console.WriteLine("No settings file found. File created with default settings.");
                Console.WriteLine("Please fill in the apikey value with your Yandex API key.");
                Console.WriteLine("Press any key to exit.");
                Console.ReadKey();
                Environment.Exit(0);
            }

            return string.Empty;
        }

        static string TranslateRussian(string russian)
        {
            return wrapper.TranslateText(russian, "ru-en").GetAwaiter().GetResult();
        }

        static string UnformatString(string text)
        {
            text = text.Replace(@"\n","#T_NL#");
            text = text.Replace(@"\'", "#T_SQ#");
            text = text.Replace("\\\"", "#T_DQ#");
            return text;
        }

        static string FormatString(string text)
        {
            text = text.Replace("#T_NL#", @"\n");
            text = text.Replace("#T_SQ#", @"\'");
            text = text.Replace("#T_DQ#", "\\\"");
            return text;
        }

        static string FirstPass(string text)
        {
            //Issue: Using & without a proper escape sequence to follow it breaks the xml parser
            //Fix: Replace &'s with and
            text = text.Replace("& ", "and ");

            //Issue: Using >> inside of <text> blocks breaks the xml parser
            //Fix: Replace >> with --
            text = text.Replace(">>", "--");

            //Issue: Newlines are used within some xml <text></text> tags
            //Fix: Look for <text> ... </text> substrings and remove the newlines
            const string openText = "<text>";
            const string closeText = "</text>";

            int posOpenText = 0;
            int posCloseText = 0;

            for (int ii = 0; ii < text.Length; ii++)
            {
                if (ii + closeText.Length < text.Length)
                {
                    if (text.Substring(ii, openText.Length) == openText)
                        posOpenText = ii; //found the open <text> tag
                    else if (text.Substring(ii, closeText.Length) == closeText)
                        posCloseText = ii; //found the close </text> tag
                }

                //if they're both found
                if (posOpenText != 0 && posCloseText != 0)
                {
                    string tagBlock = text.Substring(posOpenText, (posCloseText + closeText.Length) - posOpenText);   //get the tags and the text between them
                    text = text.Replace(tagBlock, tagBlock.Replace("\r\n", string.Empty));
                    posOpenText = 0;
                    posCloseText = 0;
                }
            }

            return text;
        }

        static string ReplaceFirst(string text, string search, string replace)
        {
            int pos = text.IndexOf(search);
            if (pos < 0)
            {
                return text;
            }
            return text.Substring(0, pos) + replace + text.Substring(pos + search.Length);
        }
    }
}

