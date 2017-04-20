import scrapy
from wikipedia.items import WikipediaItem
from bs4 import BeautifulSoup

import re


class TestLinkSpider(scrapy.Spider):
    name = 'test'
    visited_urls = set()
    start_urls = ['https://en.wikipedia.org/wiki/Minsk', 'https://en.wikipedia.org/wiki/San_Francisco', 'https://en.wikipedia.org/wiki/Hahn']

    header_selector = 'h1#firstHeading.firstHeading::text'
    body_link_selector = '(//div[@id="mw-content-text"]/p/a/@href)[position() < 100]'
    allowed_re = re.compile('https://.+.wikipedia\.org/wiki/'
                            '(?!((File|Talk|Category|Portal|Special|'
                            'Template|Template_talk|Wikipedia|Help|Draft):|Main_Page)).+')

    def parse(self, response):
        links = [link for link in response.xpath(self.body_link_selector).extract() if link[0] != '#']
        item = WikipediaItem()
        item['title'] = response.css(self.header_selector).extract_first()
        item['url'] = response.url
        item['snippet'] = BeautifulSoup(response.xpath('//div[@id="mw-content-text"]/p[1]').extract_first(), "lxml").text[:255] + "..."
        item['links'] = links
        yield item

        self.visited_urls.add(response.url)

        for link in links:
            next_url = response.urljoin(link)
            if self.allowed_re.match(next_url) and not next_url in self.visited_urls:
                yield scrapy.Request(next_url, callback=self.parse)
