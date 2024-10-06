

double getBatVolNew() //即时的电压
{
  pinMode(17, OUTPUT);
  digitalWrite(17, 1);
  delay(1);
  float vcc_cache = 0.0;
  for (uint8_t i = 0; i < 30; i++)
  {
    vcc_cache += analogRead(batpin) * 0.0009765625 * 5.607;
  }
  digitalWrite(17, 0); //关闭电池测量
  pinMode(17, INPUT);  //改为输入状态避免漏电
  String s=String(vcc_cache / 30);
  //centerx(s,50);
  Serial.print("电压：");
  Serial.println(vcc_cache / 30);
  String s1=String(getBatVolBfb((vcc_cache / 30)));
  
  s1=getBatVolBfb((vcc_cache / 30));
  if(getBatVolBfb((vcc_cache / 30))==100)s1=s1.substring(0,3)+"%";
  else if(getBatVolBfb((vcc_cache / 30))<100&&getBatVolBfb((vcc_cache / 30))>=10) s1=s1.substring(0,2)+"%";
  else if(getBatVolBfb((vcc_cache / 30))<10)s1=s1.substring(0,3)+"%";
  u8g2Fonts.setFont(u8g2_font_pcsenior_8f);
  centerx(s1, 7, 1, 15,40);
  u8g2Fonts.setFont(chinese_gb2312);
  return getBatVolBfb((vcc_cache / 30));
  
  
}
double getBatVolBfb(float batVcc) //获取电压的百分比，经过换算并非线性关系
{
  double bfb = 0.0;
  //y = 497.50976 x4 - 7,442.07254 x3 + 41,515.70648 x2 - 102,249.34377 x + 93,770.99821
  bfb = 497.50976 * batVcc * batVcc * batVcc * batVcc
        - 7442.07254 * batVcc * batVcc * batVcc
        + 41515.70648 * batVcc * batVcc
        - 102249.34377 * batVcc
        + 93770.99821;
  if (bfb > 100) bfb = 100.0;
  else if (bfb < 0) bfb = 3.0;
  return bfb;
}

